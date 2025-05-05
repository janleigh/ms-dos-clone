#include "vga.h"
#include "string.h"
#include "io.h"
#include "types.h"

unsigned short* vga_buffer;
int vga_cursor_x;
int vga_cursor_y;
unsigned char vga_color;

unsigned char vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

unsigned short vga_entry(unsigned char c, unsigned char color) {
    return (unsigned short) c | (unsigned short) color << 8;
}

void vga_update_cursor(void) {
    // Calculate cursor position
    unsigned short pos = vga_cursor_y * VGA_WIDTH + vga_cursor_x;
    
    // Set cursor position via VGA registers
    outb(VGA_CRTC_ADDR_REG, VGA_CURSOR_LOC_LOW);
    outb(VGA_CRTC_DATA_REG, (unsigned char)(pos & 0xFF));
    outb(VGA_CRTC_ADDR_REG, VGA_CURSOR_LOC_HIGH);
    outb(VGA_CRTC_DATA_REG, (unsigned char)((pos >> 8) & 0xFF));
}

void vga_init(void) {
    vga_buffer = (unsigned short*) VGA_BUFFER;
    vga_cursor_x = 0;
    vga_cursor_y = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    vga_clear_screen();
    vga_update_cursor();
}

void vga_set_color(enum vga_color fg, enum vga_color bg) {
    vga_color = vga_entry_color(fg, bg);
}

void vga_clear_screen(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
    
    vga_cursor_x = 0;
    vga_cursor_y = 0;
    vga_update_cursor();
}

void vga_scroll(void) {
    // Move everything up one line
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int dst_index = y * VGA_WIDTH + x;
            const int src_index = (y + 1) * VGA_WIDTH + x;
            vga_buffer[dst_index] = vga_buffer[src_index];
        }
    }
    
    // Clear the last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        const int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga_buffer[index] = vga_entry(' ', vga_color);
    }
    
    // Move cursor up
    vga_cursor_y--;
    vga_update_cursor();
}

// Basic character printing function
void vga_putchar(char c) {
    if (c == '\n') {
        vga_cursor_x = 0;
        vga_cursor_y++;
        
        // Check if we need to scroll
        if (vga_cursor_y >= VGA_HEIGHT) {
            vga_scroll();
        }
        
        vga_update_cursor();
        return;
    }
    
    // Handle carriage return
    if (c == '\r') {
        vga_cursor_x = 0;
        vga_update_cursor();
        return;
    }
    
    // Write character to VGA buffer
    int index = vga_cursor_y * VGA_WIDTH + vga_cursor_x;
    vga_buffer[index] = vga_entry(c, vga_color);
    
    // Move cursor
    vga_cursor_x++;
    if (vga_cursor_x >= VGA_WIDTH) {
        vga_cursor_x = 0;
        vga_cursor_y++;
        
        // Check if we need to scroll
        if (vga_cursor_y >= VGA_HEIGHT) {
            vga_scroll();
        }
    }
    
    vga_update_cursor();
}

void vga_write(const char* data, int size) {
    for (int i = 0; i < size; i++) {
        vga_putchar(data[i]);
    }
}

void vga_print(const char* str) {
    vga_write(str, strlen(str));
}

void vga_println(const char* str) {
    vga_print(str);
    vga_putchar('\n');
}