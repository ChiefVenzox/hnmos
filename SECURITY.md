# Security Policy

## Supported Versions

HNMos is an experimental operating-system project under active development. Security fixes are provided only for the latest revision of the `main` branch. Older commits, branches, demo images, and release artifacts are not supported.

| Version | Supported |
| --- | --- |
| Latest `main` | Yes |
| Older revisions and artifacts | No |

## Reporting a Vulnerability

Please do not report suspected vulnerabilities in public GitHub issues, discussions, pull requests, or wiki pages.

Report vulnerabilities privately through the repository's **Security** tab by opening a private vulnerability report or security advisory. Include:

- a clear description of the issue and its impact;
- the affected commit, branch, or artifact;
- reproducible steps or a minimal proof of concept;
- relevant logs, configuration, and environment details;
- any suggested mitigation, if available.

Do not include real API keys, access tokens, credentials, or other secrets in the report. Replace sensitive values with redacted examples.

Maintainers will acknowledge a valid report when it is reviewed, investigate its scope, and coordinate a fix and disclosure timeline when appropriate. Response times are best-effort while the project remains experimental.

## Security Scope

High-priority reports include:

- bypasses of the AI workspace or path policy;
- exposure or persistence of provider credentials;
- unsafe handling of provider-controlled serial data;
- memory-safety issues in kernel, driver, filesystem, or protocol code;
- privilege-boundary violations between the host bridge and HNMos;
- boot-image or build-pipeline integrity issues.

General feature requests, build support questions, and non-security bugs should use the public issue tracker.

## Development Security Guidelines

- Never commit credentials or embed them in kernel or boot artifacts.
- Treat AI-provider output and host-bridge input as untrusted data.
- Keep generated writes limited to the explicitly writable HNLang workspace.
- Validate paths, sizes, framing, and policy decisions before applying external data.
- Add focused tests for changes to security boundaries, permissions, or protocols.
