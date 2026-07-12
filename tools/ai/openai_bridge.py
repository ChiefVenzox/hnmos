#!/usr/bin/env python3
"""COM2-to-OpenAI Responses API bridge for the HNMos QEMU development runtime."""

from __future__ import annotations

import argparse
import json
import os
import socket
import stat
import sys
import urllib.error
import urllib.request


PROTOCOL_PREFIX = "HNAI1"
DEFAULT_API_URL = "https://api.openai.com/v1/responses"
DEFAULT_MODEL = "gpt-5.4-mini"
MAX_FRAME_BYTES = 8192
MAX_HTTP_RESPONSE_BYTES = 262144
MAX_INPUT_BYTES = 512
MAX_PROFILE_BYTES = 512
MAX_API_KEY_BYTES = 256
MAX_OUTPUT_BYTES = 900
BRIDGE_POLICY = (
    "You are the bounded HNMos HNLang coding assistant. Return only a compact HNLang "
    "workspace draft. Never request or claim kernel, boot, policy, driver, audit, raw disk, "
    "or hardware authority. Do not emit shell commands. The result is an untrusted draft "
    "that HNMos policy will validate before storing under /workspace."
)


class ProtocolError(ValueError):
    pass


def decode_hex(value: str, limit: int) -> str:
    if len(value) % 2:
        raise ProtocolError("odd-length hex field")
    try:
        raw = bytes.fromhex(value)
    except ValueError as exc:
        raise ProtocolError("invalid hex field") from exc
    if len(raw) > limit:
        raise ProtocolError("field exceeds protocol limit")
    return raw.decode("utf-8", errors="replace")


def encode_hex(value: str, limit: int = MAX_OUTPUT_BYTES) -> str:
    raw = value.encode("ascii", errors="replace")[:limit]
    return raw.hex().upper()


def parse_request(line: bytes) -> tuple[str, int, str, str]:
    if len(line) > MAX_FRAME_BYTES:
        raise ProtocolError("frame too large")
    try:
        text = line.decode("ascii").rstrip("\r\n")
    except UnicodeDecodeError as exc:
        raise ProtocolError("frame must be ASCII") from exc

    fields = text.split("|")
    if len(fields) != 6 or fields[0] != PROTOCOL_PREFIX or fields[1] != "REQ":
        raise ProtocolError("invalid request frame")
    if len(fields[2]) != 8 or len(fields[3]) != 2:
        raise ProtocolError("invalid request identifiers")

    try:
        request_id = int(fields[2], 16)
        task_type = int(fields[3], 16)
    except ValueError as exc:
        raise ProtocolError("invalid request identifiers") from exc

    prompt = decode_hex(fields[4], MAX_INPUT_BYTES)
    profile = decode_hex(fields[5], MAX_PROFILE_BYTES)
    if not prompt.strip():
        raise ProtocolError("empty prompt")
    return f"{request_id:08X}", task_type, prompt, profile


def response_frame(request_id: str, success: bool, text: str) -> bytes:
    status = "S" if success else "E"
    return f"{PROTOCOL_PREFIX}|RES|{request_id}|{status}|{encode_hex(text)}\n".encode("ascii")


def extract_output(payload: dict) -> str:
    parts: list[str] = []
    for item in payload.get("output", []):
        if not isinstance(item, dict) or item.get("type") != "message":
            continue
        for content in item.get("content", []):
            if isinstance(content, dict) and content.get("type") == "output_text":
                text = content.get("text")
                if isinstance(text, str):
                    parts.append(text)
    if not parts and isinstance(payload.get("output_text"), str):
        parts.append(payload["output_text"])
    result = "\n".join(parts).strip()
    if not result:
        raise RuntimeError("provider returned no text output")
    return result


def parse_api_key(line: bytes) -> str:
    try:
        text = line.decode("ascii").rstrip("\r\n")
    except UnicodeDecodeError as exc:
        raise ProtocolError("key frame must be ASCII") from exc
    fields = text.split("|")
    if len(fields) != 3 or fields[0] != PROTOCOL_PREFIX or fields[1] != "KEY":
        raise ProtocolError("invalid key frame")
    api_key = decode_hex(fields[2], MAX_API_KEY_BYTES)
    if len(api_key) < 8:
        raise ProtocolError("API key is too short")
    return api_key


def call_openai(prompt: str, profile: str, api_key: str) -> str:
    if not api_key:
        raise RuntimeError("OPENAI_API_KEY is not set")

    api_url = os.environ.get("OPENAI_API_URL", DEFAULT_API_URL)
    if not api_url.startswith("https://"):
        raise RuntimeError("OPENAI_API_URL must use HTTPS")

    model = os.environ.get("OPENAI_MODEL", DEFAULT_MODEL)
    instructions = BRIDGE_POLICY
    if profile.strip():
        instructions += "\n\nHNMos language profile:\n" + profile[:MAX_PROFILE_BYTES]

    body = json.dumps(
        {
            "model": model,
            "instructions": instructions,
            "input": prompt,
            "max_output_tokens": 500,
            "store": False,
        }
    ).encode("utf-8")
    request = urllib.request.Request(
        api_url,
        data=body,
        method="POST",
        headers={
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json",
        },
    )

    try:
        with urllib.request.urlopen(request, timeout=60) as response:
            payload = json.loads(response.read(MAX_HTTP_RESPONSE_BYTES).decode("utf-8"))
    except urllib.error.HTTPError as exc:
        detail = exc.read(1024).decode("utf-8", errors="replace")
        raise RuntimeError(f"OpenAI HTTP {exc.code}: {detail}") from exc
    except urllib.error.URLError as exc:
        raise RuntimeError(f"OpenAI connection failed: {exc.reason}") from exc
    return extract_output(payload)


def handle_frame(line: bytes, api_key: str) -> bytes:
    request_id = "00000000"
    try:
        request_id, task_type, prompt, profile = parse_request(line)
        if task_type not in {1, 2, 3, 4, 5, 6, 7}:
            raise ProtocolError("unsupported task type")
        output = call_openai(prompt, profile, api_key)
        return response_frame(request_id, True, output)
    except Exception as exc:  # The protocol must always return a bounded error frame.
        return response_frame(request_id, False, str(exc))


def remove_stale_socket(path: str) -> None:
    try:
        mode = os.lstat(path).st_mode
    except FileNotFoundError:
        return
    if not stat.S_ISSOCK(mode):
        raise RuntimeError(f"refusing to remove non-socket path: {path}")
    os.unlink(path)


def serve(socket_path: str) -> None:
    remove_stale_socket(socket_path)
    old_umask = os.umask(0o077)
    server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    try:
        server.bind(socket_path)
    finally:
        os.umask(old_umask)
    server.listen(1)
    print(f"HNMos AI bridge listening: {socket_path}", flush=True)

    try:
        connection, _ = server.accept()
        print("HNMos AI bridge connected to QEMU COM2", flush=True)
        with connection:
            pending = bytearray()
            session_api_key = os.environ.get("OPENAI_API_KEY", "")
            while True:
                chunk = connection.recv(1024)
                if not chunk:
                    break
                pending.extend(chunk)
                if len(pending) > MAX_FRAME_BYTES:
                    connection.sendall(response_frame("00000000", False, "frame too large"))
                    pending.clear()
                    continue
                while b"\n" in pending:
                    line, _, remainder = pending.partition(b"\n")
                    pending = bytearray(remainder)
                    if line.startswith(b"HNAI1|KEY|"):
                        try:
                            session_api_key = parse_api_key(line)
                            print("HNMos AI bridge received a volatile API credential", flush=True)
                        except ProtocolError as exc:
                            connection.sendall(response_frame("00000000", False, str(exc)))
                        continue
                    connection.sendall(handle_frame(line, session_api_key))
    finally:
        server.close()
        remove_stale_socket(socket_path)


def self_test() -> None:
    sample = b"HNAI1|REQ|0000002A|06|68656C6C6F|70726F66696C65"
    request_id, task_type, prompt, profile = parse_request(sample)
    assert request_id == "0000002A"
    assert task_type == 6
    assert prompt == "hello"
    assert profile == "profile"
    assert response_frame(request_id, True, "ok") == b"HNAI1|RES|0000002A|S|6F6B\n"
    payload = {"output": [{"type": "message", "content": [{"type": "output_text", "text": "done"}]}]}
    assert extract_output(payload) == "done"
    assert parse_api_key(b"HNAI1|KEY|6162636465666768") == "abcdefgh"
    original_call = globals()["call_openai"]
    try:
        globals()["call_openai"] = lambda prompt, profile, api_key: f"{prompt}:{profile}:{api_key[-2:]}"
        assert handle_frame(sample, "test-key") == response_frame(
            "0000002A", True, "hello:profile:ey"
        )
    finally:
        globals()["call_openai"] = original_call

    captured: dict[str, object] = {}

    class FakeResponse:
        def __enter__(self):
            return self

        def __exit__(self, exc_type, exc, traceback):
            return False

        def read(self, limit):
            assert limit == MAX_HTTP_RESPONSE_BYTES
            return json.dumps(payload).encode("utf-8")

    def fake_urlopen(request, timeout):
        captured["request"] = request
        captured["timeout"] = timeout
        return FakeResponse()

    original_urlopen = urllib.request.urlopen
    old_model = os.environ.get("OPENAI_MODEL")
    old_url = os.environ.get("OPENAI_API_URL")
    try:
        urllib.request.urlopen = fake_urlopen
        os.environ["OPENAI_MODEL"] = "test-model"
        os.environ["OPENAI_API_URL"] = DEFAULT_API_URL
        assert call_openai("hello", "profile", "test-api-key") == "done"
        request = captured["request"]
        request_body = json.loads(request.data.decode("utf-8"))
        assert request.full_url == DEFAULT_API_URL
        assert request.get_header("Authorization") == "Bearer test-api-key"
        assert request_body["model"] == "test-model"
        assert request_body["input"] == "hello"
        assert request_body["store"] is False
        assert "profile" in request_body["instructions"]
    finally:
        urllib.request.urlopen = original_urlopen
        if old_model is None:
            os.environ.pop("OPENAI_MODEL", None)
        else:
            os.environ["OPENAI_MODEL"] = old_model
        if old_url is None:
            os.environ.pop("OPENAI_API_URL", None)
        else:
            os.environ["OPENAI_API_URL"] = old_url
    print("openai bridge protocol self-test: ok")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--socket", help="Unix socket connected to QEMU COM2")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        self_test()
        return 0
    if not args.socket:
        parser.error("--socket is required unless --self-test is used")
    serve(args.socket)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
