#include "interrupts.h"
#include "../kernel/io.h"
#include "../kernel/log.h"

enum {
    HNM_IDT_ENTRIES = 256,
    HNM_KERNEL_CODE_SELECTOR = 0x08,
    HNM_IDT_INTERRUPT_GATE = 0x8E,
    HNM_IRQ_KEYBOARD = 1,
    HNM_IRQ_CASCADE = 2,
    HNM_IRQ_AI_SERIAL = 3,
    HNM_IRQ_MOUSE = 12,
    HNM_IRQ_KEYBOARD_VECTOR = 0x21,
    HNM_IRQ_AI_SERIAL_VECTOR = 0x23,
    HNM_IRQ_MOUSE_VECTOR = 0x2C,
    HNM_PIC1_COMMAND = 0x20,
    HNM_PIC1_DATA = 0x21,
    HNM_PIC2_COMMAND = 0xA0,
    HNM_PIC2_DATA = 0xA1,
    HNM_PIC_EOI = 0x20,
    HNM_ICW1_INIT = 0x10,
    HNM_ICW1_ICW4 = 0x01,
    HNM_ICW4_8086 = 0x01
};

struct hnm_idt_entry {
    u16 offset_low;
    u16 selector;
    u8 zero;
    u8 type_attr;
    u16 offset_high;
} __attribute__((packed));

struct hnm_idt_pointer {
    u16 limit;
    u32 base;
} __attribute__((packed));

extern void hnm_irq1_stub(void);
extern void hnm_irq3_stub(void);
extern void hnm_irq12_stub(void);

static struct hnm_idt_entry hnm_idt[HNM_IDT_ENTRIES];

static void hnm_idt_set_gate(u8 vector, u32 handler)
{
    hnm_idt[vector].offset_low = (u16)(handler & 0xFFFF);
    hnm_idt[vector].selector = HNM_KERNEL_CODE_SELECTOR;
    hnm_idt[vector].zero = 0;
    hnm_idt[vector].type_attr = HNM_IDT_INTERRUPT_GATE;
    hnm_idt[vector].offset_high = (u16)((handler >> 16) & 0xFFFF);
}

static void hnm_idt_load(void)
{
    struct hnm_idt_pointer pointer;

    pointer.limit = (u16)(sizeof(hnm_idt) - 1);
    pointer.base = (u32)hnm_idt;

    __asm__ volatile ("lidt %0" : : "m"(pointer));
}

static void hnm_pic_remap_input_irqs(void)
{
    u8 master_mask = (u8)~(
        (1 << HNM_IRQ_KEYBOARD) |
        (1 << HNM_IRQ_CASCADE) |
        (1 << HNM_IRQ_AI_SERIAL));
    u8 slave_mask = (u8)~(1 << (HNM_IRQ_MOUSE - 8));

    hnm_outb(HNM_PIC1_COMMAND, HNM_ICW1_INIT | HNM_ICW1_ICW4);
    hnm_io_wait();
    hnm_outb(HNM_PIC2_COMMAND, HNM_ICW1_INIT | HNM_ICW1_ICW4);
    hnm_io_wait();

    hnm_outb(HNM_PIC1_DATA, 0x20);
    hnm_io_wait();
    hnm_outb(HNM_PIC2_DATA, 0x28);
    hnm_io_wait();

    hnm_outb(HNM_PIC1_DATA, 0x04);
    hnm_io_wait();
    hnm_outb(HNM_PIC2_DATA, 0x02);
    hnm_io_wait();

    hnm_outb(HNM_PIC1_DATA, HNM_ICW4_8086);
    hnm_io_wait();
    hnm_outb(HNM_PIC2_DATA, HNM_ICW4_8086);
    hnm_io_wait();

    hnm_outb(HNM_PIC1_DATA, master_mask);
    hnm_outb(HNM_PIC2_DATA, slave_mask);
}

void hnm_pic_send_eoi(unsigned char irq)
{
    if (irq >= 8) {
        hnm_outb(HNM_PIC2_COMMAND, HNM_PIC_EOI);
    }

    hnm_outb(HNM_PIC1_COMMAND, HNM_PIC_EOI);
}

void hnm_interrupts_init(void)
{
    for (u32 i = 0; i < HNM_IDT_ENTRIES; i++) {
        hnm_idt[i].offset_low = 0;
        hnm_idt[i].selector = 0;
        hnm_idt[i].zero = 0;
        hnm_idt[i].type_attr = 0;
        hnm_idt[i].offset_high = 0;
    }

    hnm_idt_set_gate(HNM_IRQ_KEYBOARD_VECTOR, (u32)hnm_irq1_stub);
    hnm_idt_set_gate(HNM_IRQ_AI_SERIAL_VECTOR, (u32)hnm_irq3_stub);
    hnm_idt_set_gate(HNM_IRQ_MOUSE_VECTOR, (u32)hnm_irq12_stub);
    hnm_idt_load();
    hnm_pic_remap_input_irqs();
    hnm_log_write_line("interrupts: IDT ready, PIC IRQ1/IRQ3/IRQ12 unmasked.");
}

void hnm_interrupts_enable(void)
{
    __asm__ volatile ("sti");
}

void hnm_interrupts_disable(void)
{
    __asm__ volatile ("cli");
}

void hnm_interrupts_idle(void)
{
    for (;;) {
        __asm__ volatile ("sti; hlt");
    }
}
