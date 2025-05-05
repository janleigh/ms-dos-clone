#ifndef VGA_H
#define VGA_H

// VGA text mode buffer address
#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// VGA cursor control registers
#define VGA_CRTC_ADDR_REG 0x3D4
#define VGA_CRTC_DATA_REG 0x3D5
#define VGA_CURSOR_LOC_HIGH 0x0E
#define VGA_CURSOR_LOC_LOW 0x0F

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

extern unsigned short* vga_buffer;
extern int vga_cursor_x;
extern int vga_cursor_y;
extern unsigned char vga_color;

unsigned char vga_entry_color(enum vga_color fg, enum vga_color bg);
unsigned short vga_entry(unsigned char c, unsigned char color);
void vga_init(void);
void vga_set_color(enum vga_color fg, enum vga_color bg);
void vga_putchar(char c);
void vga_write(const char* data, int size);
void vga_print(const char* str);
void vga_println(const char* str);
void vga_clear_screen(void);
void vga_update_cursor(void);
void vga_scroll(void);

#endif