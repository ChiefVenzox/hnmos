#include "log.h"

enum {
    HNM_VGA_WIDTH = 80,
    HNM_VGA_HEIGHT = 25,
    HNM_VGA_MEMORY = 0xB8000,
    HNM_SERIAL_COM1 = 0x3F8
};

enum hnm_vga_color {
    HNM_VGA_BLACK = 0,
    HNM_VGA_LIGHT_GREEN = 10,
    HNM_VGA_LIGHT_RED = 12,
    HNM_VGA_WHITE = 15
};

static volatile u16 *const hnm_vga_buffer = (u16 *)HNM_VGA_MEMORY;
static usize hnm_vga_row;
static usize hnm_vga_column;
static u8 hnm_vga_color_byte;

static inline void hnm_outb(u16 port, u8 value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 hnm_inb(u16 port)
{
    u8 value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static u8 hnm_vga_color(enum hnm_vga_color foreground, enum hnm_vga_color background)
{
    return (u8)(foreground | (background << 4));
}

static u16 hnm_vga_entry(char character, u8 color)
{
    return (u16)((u8)character | ((u16)color << 8));
}

static void hnm_vga_clear(void)
{
    for (usize y = 0; y < HNM_VGA_HEIGHT; y++) {
        for (usize x = 0; x < HNM_VGA_WIDTH; x++) {
            hnm_vga_buffer[(y * HNM_VGA_WIDTH) + x] = hnm_vga_entry(' ', hnm_vga_color_byte);
        }
    }

    hnm_vga_row = 0;
    hnm_vga_column = 0;
}

static void hnm_vga_newline(void)
{
    hnm_vga_column = 0;

    if (hnm_vga_row + 1 < HNM_VGA_HEIGHT) {
        hnm_vga_row++;
    }
}

static void hnm_vga_put_char(char character)
{
    if (character == '\n') {
        hnm_vga_newline();
        return;
    }

    hnm_vga_buffer[(hnm_vga_row * HNM_VGA_WIDTH) + hnm_vga_column] =
        hnm_vga_entry(character, hnm_vga_color_byte);
    hnm_vga_column++;

    if (hnm_vga_column >= HNM_VGA_WIDTH) {
        hnm_vga_newline();
    }
}

static void hnm_serial_init(void)
{
    hnm_outb(HNM_SERIAL_COM1 + 1, 0x00);
    hnm_outb(HNM_SERIAL_COM1 + 3, 0x80);
    hnm_outb(HNM_SERIAL_COM1 + 0, 0x03);
    hnm_outb(HNM_SERIAL_COM1 + 1, 0x00);
    hnm_outb(HNM_SERIAL_COM1 + 3, 0x03);
    hnm_outb(HNM_SERIAL_COM1 + 2, 0xC7);
    hnm_outb(HNM_SERIAL_COM1 + 4, 0x0B);
}

static int hnm_serial_can_write(void)
{
    return (hnm_inb(HNM_SERIAL_COM1 + 5) & 0x20) != 0;
}

static void hnm_serial_write_char(char character)
{
    for (u32 attempt = 0; attempt < 100000; attempt++) {
        if (hnm_serial_can_write()) {
            hnm_outb(HNM_SERIAL_COM1, (u8)character);
            return;
        }
    }
}

static void hnm_log_put_char(char character)
{
    hnm_vga_put_char(character);

    if (character == '\n') {
        hnm_serial_write_char('\r');
    }

    hnm_serial_write_char(character);
}

void hnm_log_init(void)
{
    hnm_vga_color_byte = hnm_vga_color(HNM_VGA_LIGHT_GREEN, HNM_VGA_BLACK);
    hnm_vga_clear();
    hnm_serial_init();
}

void hnm_log_write(const char *text)
{
    for (usize i = 0; text[i] != '\0'; i++) {
        hnm_log_put_char(text[i]);
    }
}

void hnm_log_write_line(const char *text)
{
    hnm_log_write(text);
    hnm_log_put_char('\n');
}

void hnm_log_write_hex32(u32 value)
{
    const char *digits = "0123456789ABCDEF";

    hnm_log_write("0x");

    for (int shift = 28; shift >= 0; shift -= 4) {
        hnm_log_put_char(digits[(value >> shift) & 0xF]);
    }
}
