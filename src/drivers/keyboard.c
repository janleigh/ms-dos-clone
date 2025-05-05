#include "keyboard.h"
#include "io.h"
#include "vga.h"
#include "kernel.h"
#include "filesystem.h"
#include "string.h"
#include "types.h"

static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static const char scancode_to_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int extended_key = 0;
static unsigned char last_scancode = 0;

void keyboard_init(void) {
    last_scancode = 0;
    extended_key = 0;
    ctrl_pressed = 0;
}

// Check if there's a key available to read
int keyboard_is_key_available(void) {
    // Check bit 0 of the keyboard status port
    // If it's 1, there's data available to read
    return inb(KEYBOARD_STATUS_PORT) & 1;
}

unsigned char keyboard_get_scancode(void) {
    return inb(KEYBOARD_DATA_PORT);
}

char keyboard_scancode_to_ascii(unsigned char scancode) {
    // Check for extended key sequence (0xE0 followed by another scancode)
    if (scancode == 0xE0) {
        extended_key = 1;
        return 0;
    }
    
    // If this is the second byte of an extended key sequence
    if (extended_key) {
        extended_key = 0;
        
        // Handle extended keys (arrow keys)
        switch (scancode) {
            case 0x48: // Up arrow
                return KEY_UP;
            case 0x50: // Down arrow
                return KEY_DOWN;
            case 0x4B: // Left arrow
                return KEY_LEFT;
            case 0x4D: // Right arrow
                return KEY_RIGHT;
            default:
                return 0;
        }
    }
    
    // Handle modifier keys
    if (scancode == 0x2A || scancode == 0x36) {  // Left or right shift pressed
        shift_pressed = 1;
        return 0;
    } else if (scancode == 0xAA || scancode == 0xB6) {  // Left or right shift released
        shift_pressed = 0;
        return 0;
    } else if (scancode == 0x1D) {  // Left control pressed
        ctrl_pressed = 1;
        return 0;
    } else if (scancode == 0x9D) {  // Left control released
        ctrl_pressed = 0;
        return 0;
    }
    
    // Check if key is released (bit 7 is set)
    if (scancode & 0x80) {
        return 0;
    }
    
    // Tab key
    if (scancode == KEY_TAB) {
        return KEY_TAB;
    }
    
    // Regular keys
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            return scancode_to_ascii_shift[scancode];
        } else {
            return scancode_to_ascii[scancode];
        }
    }
    
    return 0;  // Unknown scancode
}

// Helper function to clear the current input line
void clear_input_line(void) {
    // Move cursor to beginning of the line
    vga_cursor_x = 3;  // After "C:> "
    
    // Erase the current line
    for (int i = 0; i < buffer_position; i++) {
        vga_putchar(' ');
    }
    
    // Reset cursor position
    vga_cursor_x = 3;
    vga_update_cursor();
}

void keyboard_handler(void) {
    // Only read if a key is actually available
    if (keyboard_is_key_available()) {
        unsigned char scancode = keyboard_get_scancode();
        
        // Only process a scancode if it's different from the last one
        // This prevents repeated processing of the same key
        if (scancode != last_scancode || (scancode & 0x80)) {
            last_scancode = scancode;
            
            char key = keyboard_scancode_to_ascii(scancode);
            
            if (key) {
                // Handle special keys
                if (key == KEY_UP) {
                    // Navigate up through command history
                    navigate_history(-1);
                }
                else if (key == KEY_DOWN) {
                    // Navigate down through command history
                    navigate_history(1);
                }
                else if (key == KEY_TAB) {
                    // Handle tab completion
                    handle_tab_completion();
                }
                // Handle backspace
                else if (key == '\b') {
                    if (buffer_position > 0) {
                        buffer_position--;
                        vga_cursor_x--;
                        if (vga_cursor_x < 0) {
                            vga_cursor_x = VGA_WIDTH - 1;
                            vga_cursor_y--;
                        }
                        const int index = vga_cursor_y * VGA_WIDTH + vga_cursor_x;
                        vga_buffer[index] = vga_entry(' ', vga_color);
                        vga_update_cursor();
                    }
                }
                // Handle enter key
                else if (key == '\n') {
                    input_buffer[buffer_position] = '\0';
                    
                    // Add command to history if it's not empty
                    if (buffer_position > 0) {
                        add_to_history(input_buffer);
                    }
                    
                    process_command();
                    buffer_position = 0;
                    history_position = -1;
                }
                // Handle regular characters
                else if (buffer_position < MAX_COMMAND_LENGTH - 1) {
                    input_buffer[buffer_position++] = key;
                    vga_putchar(key);
                }
            }
        }
    }
}