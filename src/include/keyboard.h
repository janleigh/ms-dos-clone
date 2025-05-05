#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEY_UP      0x48
#define KEY_DOWN    0x50
#define KEY_LEFT    0x4B
#define KEY_RIGHT   0x4D
#define KEY_TAB     0x0F
#define KEY_ESCAPE  0x01

void keyboard_init(void);
void keyboard_handler(void);
int keyboard_is_key_available(void);
unsigned char keyboard_get_scancode(void);
char keyboard_scancode_to_ascii(unsigned char scancode);
void clear_input_line(void);

#endif