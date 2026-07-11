#include "ai_serial_transport.h"
#include "../io.h"
#include "../log.h"
#include "../../interrupts/interrupts.h"

enum {
    HNM_AI_SERIAL_COM2 = 0x2F8,
    HNM_AI_SERIAL_IRQ = 3,
    HNM_AI_SERIAL_RX_CAPACITY = 4096,
    HNM_AI_SERIAL_LINE_CAPACITY = 2304
};

static volatile char hnm_ai_serial_rx[HNM_AI_SERIAL_RX_CAPACITY];
static volatile u32 hnm_ai_serial_rx_head;
static volatile u32 hnm_ai_serial_rx_tail;
static volatile u32 hnm_ai_serial_rx_dropped;
static char hnm_ai_serial_line[HNM_AI_SERIAL_LINE_CAPACITY];
static u32 hnm_ai_serial_line_length;
static int hnm_ai_serial_ready;

static int hnm_ai_serial_can_write(void)
{
    return (hnm_inb(HNM_AI_SERIAL_COM2 + 5) & 0x20) != 0;
}

static void hnm_ai_serial_write_char(char character)
{
    for (u32 attempt = 0; attempt < 100000; attempt++) {
        if (hnm_ai_serial_can_write()) {
            hnm_outb(HNM_AI_SERIAL_COM2, (u8)character);
            return;
        }
    }
}

static void hnm_ai_serial_write_text(const char *text)
{
    if (text == 0) {
        return;
    }

    for (u32 i = 0; text[i] != '\0'; i++) {
        hnm_ai_serial_write_char(text[i]);
    }
}

static char hnm_ai_serial_hex_digit(u8 value)
{
    if (value < 10) {
        return (char)('0' + value);
    }

    return (char)('A' + (value - 10));
}

static int hnm_ai_serial_hex_value(char character)
{
    if (character >= '0' && character <= '9') {
        return character - '0';
    }

    if (character >= 'A' && character <= 'F') {
        return 10 + (character - 'A');
    }

    if (character >= 'a' && character <= 'f') {
        return 10 + (character - 'a');
    }

    return -1;
}

static void hnm_ai_serial_write_hex_byte(u8 value)
{
    hnm_ai_serial_write_char(hnm_ai_serial_hex_digit((u8)(value >> 4)));
    hnm_ai_serial_write_char(hnm_ai_serial_hex_digit((u8)(value & 0x0F)));
}

static void hnm_ai_serial_write_hex_u32(u32 value)
{
    for (int shift = 28; shift >= 0; shift -= 4) {
        hnm_ai_serial_write_char(hnm_ai_serial_hex_digit((u8)((value >> shift) & 0x0F)));
    }
}

static void hnm_ai_serial_write_hex_text(const char *text)
{
    if (text == 0) {
        return;
    }

    for (u32 i = 0; text[i] != '\0'; i++) {
        hnm_ai_serial_write_hex_byte((u8)text[i]);
    }
}

static void hnm_ai_serial_push_rx(char character)
{
    u32 next_head = (hnm_ai_serial_rx_head + 1) % HNM_AI_SERIAL_RX_CAPACITY;

    if (next_head == hnm_ai_serial_rx_tail) {
        hnm_ai_serial_rx_dropped++;
        return;
    }

    hnm_ai_serial_rx[hnm_ai_serial_rx_head] = character;
    hnm_ai_serial_rx_head = next_head;
}

static int hnm_ai_serial_pop_rx(char *character)
{
    if (hnm_ai_serial_rx_tail == hnm_ai_serial_rx_head || character == 0) {
        return 0;
    }

    *character = hnm_ai_serial_rx[hnm_ai_serial_rx_tail];
    hnm_ai_serial_rx_tail = (hnm_ai_serial_rx_tail + 1) % HNM_AI_SERIAL_RX_CAPACITY;
    return 1;
}

static void hnm_ai_serial_capture_rx(void)
{
    while ((hnm_inb(HNM_AI_SERIAL_COM2 + 5) & 0x01) != 0) {
        hnm_ai_serial_push_rx((char)hnm_inb(HNM_AI_SERIAL_COM2));
    }
}

static int hnm_ai_serial_prefix_matches(const char *line, const char *prefix)
{
    u32 index = 0;

    while (prefix[index] != '\0') {
        if (line[index] != prefix[index]) {
            return 0;
        }

        index++;
    }

    return 1;
}

static int hnm_ai_serial_parse_u32(const char *text, u32 *value)
{
    u32 result = 0;

    if (text == 0 || value == 0) {
        return 0;
    }

    for (u32 i = 0; i < 8; i++) {
        int digit = hnm_ai_serial_hex_value(text[i]);

        if (digit < 0) {
            return 0;
        }

        result = (result << 4) | (u32)digit;
    }

    *value = result;
    return 1;
}

static int hnm_ai_serial_decode_text(const char *encoded, char *decoded, u32 capacity)
{
    u32 output_index = 0;

    if (encoded == 0 || decoded == 0 || capacity == 0) {
        return 0;
    }

    for (u32 i = 0; encoded[i] != '\0'; i += 2) {
        int high;
        int low;

        if (encoded[i + 1] == '\0') {
            return 0;
        }

        high = hnm_ai_serial_hex_value(encoded[i]);
        low = hnm_ai_serial_hex_value(encoded[i + 1]);

        if (high < 0 || low < 0) {
            return 0;
        }

        if (output_index + 1 < capacity) {
            decoded[output_index] = (char)((high << 4) | low);
            output_index++;
        }
    }

    decoded[output_index] = '\0';
    return 1;
}

static int hnm_ai_serial_parse_response(struct hnm_ai_task *task)
{
    static char decoded[HNM_AI_TASK_OUTPUT_MAX];
    u32 task_id;

    if (task == 0 || !hnm_ai_serial_prefix_matches(hnm_ai_serial_line, "HNAI1|RES|") ||
        !hnm_ai_serial_parse_u32(&hnm_ai_serial_line[10], &task_id) ||
        hnm_ai_serial_line[18] != '|' || hnm_ai_serial_line[20] != '|' ||
        task_id != task->task_id ||
        !hnm_ai_serial_decode_text(
            &hnm_ai_serial_line[21],
            decoded,
            sizeof(decoded))) {
        return 0;
    }

    hnm_ai_task_set_output(task, decoded);

    if (hnm_ai_serial_line[19] == 'S') {
        task->status = AI_TASK_DONE;
    } else {
        task->status = AI_TASK_ERROR;
    }

    return 1;
}

void hnm_ai_serial_transport_init(void)
{
    hnm_ai_serial_rx_head = 0;
    hnm_ai_serial_rx_tail = 0;
    hnm_ai_serial_rx_dropped = 0;
    hnm_ai_serial_line_length = 0;

    hnm_outb(HNM_AI_SERIAL_COM2 + 1, 0x00);
    hnm_outb(HNM_AI_SERIAL_COM2 + 3, 0x80);
    hnm_outb(HNM_AI_SERIAL_COM2 + 0, 0x03);
    hnm_outb(HNM_AI_SERIAL_COM2 + 1, 0x00);
    hnm_outb(HNM_AI_SERIAL_COM2 + 3, 0x03);
    hnm_outb(HNM_AI_SERIAL_COM2 + 2, 0xC7);
    hnm_outb(HNM_AI_SERIAL_COM2 + 4, 0x0B);
    hnm_outb(HNM_AI_SERIAL_COM2 + 1, 0x01);

    hnm_ai_serial_ready = 1;
    hnm_log_write_line("ai transport: COM2 framed bridge ready.");
}

int hnm_ai_serial_transport_send(const struct hnm_ai_task *task)
{
    if (!hnm_ai_serial_ready || task == 0 || task->input[0] == '\0') {
        return 0;
    }

    hnm_ai_serial_write_text("HNAI1|REQ|");
    hnm_ai_serial_write_hex_u32(task->task_id);
    hnm_ai_serial_write_char('|');
    hnm_ai_serial_write_hex_byte((u8)task->task_type);
    hnm_ai_serial_write_char('|');
    hnm_ai_serial_write_hex_text(task->input);
    hnm_ai_serial_write_char('|');
    hnm_ai_serial_write_hex_text(task->language_profile);
    hnm_ai_serial_write_char('\n');
    return 1;
}

int hnm_ai_serial_transport_send_api_key(const char *api_key)
{
    if (!hnm_ai_serial_ready || api_key == 0 || api_key[0] == '\0') {
        return 0;
    }

    hnm_ai_serial_write_text("HNAI1|KEY|");
    hnm_ai_serial_write_hex_text(api_key);
    hnm_ai_serial_write_char('\n');
    return 1;
}

int hnm_ai_serial_transport_poll(struct hnm_ai_task *task)
{
    char character;

    if (!hnm_ai_serial_ready || task == 0) {
        return 0;
    }

    hnm_ai_serial_capture_rx();

    while (hnm_ai_serial_pop_rx(&character)) {
        if (character == '\r') {
            continue;
        }

        if (character == '\n') {
            hnm_ai_serial_line[hnm_ai_serial_line_length] = '\0';
            hnm_ai_serial_line_length = 0;

            if (hnm_ai_serial_parse_response(task)) {
                return 1;
            }

            continue;
        }

        if (hnm_ai_serial_line_length + 1 < sizeof(hnm_ai_serial_line)) {
            hnm_ai_serial_line[hnm_ai_serial_line_length] = character;
            hnm_ai_serial_line_length++;
        } else {
            hnm_ai_serial_line_length = 0;
        }
    }

    return 0;
}

void hnm_ai_serial_irq_handler(void)
{
    hnm_ai_serial_capture_rx();
    hnm_pic_send_eoi(HNM_AI_SERIAL_IRQ);
}

u32 hnm_ai_serial_dropped_bytes(void)
{
    return hnm_ai_serial_rx_dropped;
}
