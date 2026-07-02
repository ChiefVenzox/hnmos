#include "framebuffer.h"
#include "../log.h"

enum {
    HNM_MULTIBOOT_INFO_FRAMEBUFFER = 1 << 12,
    HNM_MULTIBOOT_FRAMEBUFFER_TYPE_RGB = 1,
    HNM_BGA_INDEX_PORT = 0x01CE,
    HNM_BGA_DATA_PORT = 0x01CF,
    HNM_BGA_INDEX_ID = 0,
    HNM_BGA_INDEX_XRES = 1,
    HNM_BGA_INDEX_YRES = 2,
    HNM_BGA_INDEX_BPP = 3,
    HNM_BGA_INDEX_ENABLE = 4,
    HNM_BGA_INDEX_VIRT_WIDTH = 6,
    HNM_BGA_INDEX_VIRT_HEIGHT = 7,
    HNM_BGA_INDEX_X_OFFSET = 8,
    HNM_BGA_INDEX_Y_OFFSET = 9,
    HNM_BGA_ID0 = 0xB0C0,
    HNM_BGA_ID5 = 0xB0C5,
    HNM_BGA_ENABLED = 0x01,
    HNM_BGA_LFB_ENABLED = 0x40,
    HNM_BGA_WIDTH = 1024,
    HNM_BGA_HEIGHT = 768,
    HNM_BGA_BPP = 32,
    HNM_PCI_CONFIG_ADDRESS = 0x0CF8,
    HNM_PCI_CONFIG_DATA = 0x0CFC,
    HNM_PCI_ENABLE = 0x80000000,
    HNM_PCI_VENDOR_NONE = 0xFFFF,
    HNM_PCI_CLASS_DISPLAY = 0x03
};

struct hnm_multiboot_info {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_addr;
    u32 syms[4];
    u32 mmap_length;
    u32 mmap_addr;
    u32 drives_length;
    u32 drives_addr;
    u32 config_table;
    u32 boot_loader_name;
    u32 apm_table;
    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
    u32 framebuffer_addr_low;
    u32 framebuffer_addr_high;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8 framebuffer_bpp;
    u8 framebuffer_type;
    u8 color_info[6];
} __attribute__((packed));

static struct hnm_framebuffer hnm_fb;

static inline void hnm_outw(u16 port, u16 value)
{
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline u16 hnm_inw(u16 port)
{
    u16 value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void hnm_outl(u16 port, u32 value)
{
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline u32 hnm_inl(u16 port)
{
    u32 value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void hnm_bga_write(u16 index, u16 value)
{
    hnm_outw(HNM_BGA_INDEX_PORT, index);
    hnm_outw(HNM_BGA_DATA_PORT, value);
}

static u16 hnm_bga_read(u16 index)
{
    hnm_outw(HNM_BGA_INDEX_PORT, index);
    return hnm_inw(HNM_BGA_DATA_PORT);
}

static u32 hnm_pci_read_config(u8 bus, u8 device, u8 function, u8 offset)
{
    u32 address = HNM_PCI_ENABLE |
        ((u32)bus << 16) |
        ((u32)device << 11) |
        ((u32)function << 8) |
        ((u32)offset & 0xFC);

    hnm_outl(HNM_PCI_CONFIG_ADDRESS, address);
    return hnm_inl(HNM_PCI_CONFIG_DATA);
}

static u32 hnm_bga_find_lfb_address(void)
{
    for (u8 device = 0; device < 32; device++) {
        u32 id = hnm_pci_read_config(0, device, 0, 0x00);
        u32 class_reg;
        u8 class_code;

        if ((id & 0xFFFF) == HNM_PCI_VENDOR_NONE) {
            continue;
        }

        class_reg = hnm_pci_read_config(0, device, 0, 0x08);
        class_code = (u8)(class_reg >> 24);

        if (class_code == HNM_PCI_CLASS_DISPLAY) {
            u32 bar0 = hnm_pci_read_config(0, device, 0, 0x10);

            if ((bar0 & 0x1) == 0) {
                return bar0 & 0xFFFFFFF0;
            }
        }
    }

    return 0;
}

static int hnm_framebuffer_supported_bpp(u8 bpp)
{
    return bpp == 16 || bpp == 24 || bpp == 32;
}

static void hnm_framebuffer_set_default_masks(void)
{
    if (hnm_fb.bpp == 16) {
        hnm_fb.red_position = 11;
        hnm_fb.red_mask_size = 5;
        hnm_fb.green_position = 5;
        hnm_fb.green_mask_size = 6;
        hnm_fb.blue_position = 0;
        hnm_fb.blue_mask_size = 5;
        return;
    }

    hnm_fb.red_position = 16;
    hnm_fb.red_mask_size = 8;
    hnm_fb.green_position = 8;
    hnm_fb.green_mask_size = 8;
    hnm_fb.blue_position = 0;
    hnm_fb.blue_mask_size = 8;
}

static int hnm_framebuffer_init_multiboot(u32 multiboot_info_addr)
{
    const struct hnm_multiboot_info *info = (const struct hnm_multiboot_info *)multiboot_info_addr;

    if (multiboot_info_addr == 0) {
        return 0;
    }

    if ((info->flags & HNM_MULTIBOOT_INFO_FRAMEBUFFER) == 0) {
        return 0;
    }

    if (info->framebuffer_addr_high != 0) {
        return 0;
    }

    if (info->framebuffer_type != HNM_MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {
        return 0;
    }

    if (!hnm_framebuffer_supported_bpp(info->framebuffer_bpp)) {
        return 0;
    }

    if (info->framebuffer_addr_low == 0 ||
        info->framebuffer_width == 0 ||
        info->framebuffer_height == 0 ||
        info->framebuffer_pitch == 0) {
        return 0;
    }

    hnm_fb.buffer = (volatile u8 *)info->framebuffer_addr_low;
    hnm_fb.address = info->framebuffer_addr_low;
    hnm_fb.width = info->framebuffer_width;
    hnm_fb.height = info->framebuffer_height;
    hnm_fb.pitch = info->framebuffer_pitch;
    hnm_fb.bpp = info->framebuffer_bpp;
    hnm_fb.type = info->framebuffer_type;
    hnm_fb.red_position = info->color_info[0];
    hnm_fb.red_mask_size = info->color_info[1];
    hnm_fb.green_position = info->color_info[2];
    hnm_fb.green_mask_size = info->color_info[3];
    hnm_fb.blue_position = info->color_info[4];
    hnm_fb.blue_mask_size = info->color_info[5];
    hnm_fb.source = HNM_FRAMEBUFFER_SOURCE_MULTIBOOT;

    if (hnm_fb.red_mask_size == 0 || hnm_fb.green_mask_size == 0 || hnm_fb.blue_mask_size == 0) {
        hnm_framebuffer_set_default_masks();
    }

    hnm_fb.available = 1;
    return 1;
}

static int hnm_framebuffer_init_bga(void)
{
    u16 id = hnm_bga_read(HNM_BGA_INDEX_ID);
    u32 lfb_address;

    if (id < HNM_BGA_ID0 || id > HNM_BGA_ID5) {
        return 0;
    }

    lfb_address = hnm_bga_find_lfb_address();
    if (lfb_address == 0) {
        return 0;
    }

    hnm_bga_write(HNM_BGA_INDEX_ENABLE, 0);
    hnm_bga_write(HNM_BGA_INDEX_XRES, HNM_BGA_WIDTH);
    hnm_bga_write(HNM_BGA_INDEX_YRES, HNM_BGA_HEIGHT);
    hnm_bga_write(HNM_BGA_INDEX_VIRT_WIDTH, HNM_BGA_WIDTH);
    hnm_bga_write(HNM_BGA_INDEX_VIRT_HEIGHT, HNM_BGA_HEIGHT);
    hnm_bga_write(HNM_BGA_INDEX_X_OFFSET, 0);
    hnm_bga_write(HNM_BGA_INDEX_Y_OFFSET, 0);
    hnm_bga_write(HNM_BGA_INDEX_BPP, HNM_BGA_BPP);
    hnm_bga_write(HNM_BGA_INDEX_ENABLE, HNM_BGA_ENABLED | HNM_BGA_LFB_ENABLED);

    hnm_fb.buffer = (volatile u8 *)lfb_address;
    hnm_fb.address = lfb_address;
    hnm_fb.width = HNM_BGA_WIDTH;
    hnm_fb.height = HNM_BGA_HEIGHT;
    hnm_fb.pitch = HNM_BGA_WIDTH * (HNM_BGA_BPP / 8);
    hnm_fb.bpp = HNM_BGA_BPP;
    hnm_fb.type = HNM_MULTIBOOT_FRAMEBUFFER_TYPE_RGB;
    hnm_fb.source = HNM_FRAMEBUFFER_SOURCE_BGA;
    hnm_framebuffer_set_default_masks();
    hnm_fb.available = 1;
    return 1;
}

int hnm_framebuffer_init(u32 multiboot_info_addr)
{
    hnm_fb.available = 0;
    hnm_fb.source = HNM_FRAMEBUFFER_SOURCE_NONE;

    if (hnm_framebuffer_init_multiboot(multiboot_info_addr)) {
        return 1;
    }

    return hnm_framebuffer_init_bga();
}

int hnm_framebuffer_is_available(void)
{
    return hnm_fb.available;
}

const struct hnm_framebuffer *hnm_framebuffer_get(void)
{
    return &hnm_fb;
}

void hnm_framebuffer_log_info(void)
{
    if (!hnm_fb.available) {
        hnm_log_write_line("graphics: framebuffer unavailable.");
        return;
    }

    hnm_log_write_line("graphics: framebuffer ready.");
    hnm_log_write("framebuffer source:  ");
    if (hnm_fb.source == HNM_FRAMEBUFFER_SOURCE_MULTIBOOT) {
        hnm_log_write_line("multiboot");
    } else if (hnm_fb.source == HNM_FRAMEBUFFER_SOURCE_BGA) {
        hnm_log_write_line("qemu-bga");
    } else {
        hnm_log_write_line("unknown");
    }
    hnm_log_write("framebuffer address: ");
    hnm_log_write_hex32(hnm_fb.address);
    hnm_log_write_line("");
    hnm_log_write("framebuffer width:   ");
    hnm_log_write_hex32(hnm_fb.width);
    hnm_log_write_line("");
    hnm_log_write("framebuffer height:  ");
    hnm_log_write_hex32(hnm_fb.height);
    hnm_log_write_line("");
    hnm_log_write("framebuffer pitch:   ");
    hnm_log_write_hex32(hnm_fb.pitch);
    hnm_log_write_line("");
    hnm_log_write("framebuffer bpp:     ");
    hnm_log_write_hex32(hnm_fb.bpp);
    hnm_log_write_line("");
}

static u32 hnm_framebuffer_component(u8 value, u8 mask_size)
{
    if (mask_size == 0) {
        return 0;
    }

    if (mask_size >= 8) {
        return (u32)value << (mask_size - 8);
    }

    return (u32)(value >> (8 - mask_size));
}

static u32 hnm_framebuffer_pack_color(hnm_color color)
{
    return (hnm_framebuffer_component(hnm_color_red(color), hnm_fb.red_mask_size) << hnm_fb.red_position) |
        (hnm_framebuffer_component(hnm_color_green(color), hnm_fb.green_mask_size) << hnm_fb.green_position) |
        (hnm_framebuffer_component(hnm_color_blue(color), hnm_fb.blue_mask_size) << hnm_fb.blue_position);
}

void hnm_framebuffer_put_pixel(u32 x, u32 y, hnm_color color)
{
    volatile u8 *pixel;
    u32 packed;

    if (!hnm_fb.available || x >= hnm_fb.width || y >= hnm_fb.height) {
        return;
    }

    pixel = hnm_fb.buffer + (y * hnm_fb.pitch) + (x * ((u32)hnm_fb.bpp / 8));
    packed = hnm_framebuffer_pack_color(color);

    if (hnm_fb.bpp == 32) {
        *((volatile u32 *)pixel) = packed;
    } else if (hnm_fb.bpp == 24) {
        pixel[0] = (u8)(packed & 0xFF);
        pixel[1] = (u8)((packed >> 8) & 0xFF);
        pixel[2] = (u8)((packed >> 16) & 0xFF);
    } else if (hnm_fb.bpp == 16) {
        *((volatile u16 *)pixel) = (u16)packed;
    }
}

void hnm_framebuffer_clear(hnm_color color)
{
    if (!hnm_fb.available) {
        return;
    }

    for (u32 y = 0; y < hnm_fb.height; y++) {
        for (u32 x = 0; x < hnm_fb.width; x++) {
            hnm_framebuffer_put_pixel(x, y, color);
        }
    }
}
