#ifndef KERNEL_H
#define KERNEL_H

#define COMMAND_HISTORY_SIZE 10
#define MAX_COMMAND_LENGTH 256

extern char input_buffer[MAX_COMMAND_LENGTH];
extern int buffer_position;
extern char command_history[COMMAND_HISTORY_SIZE][MAX_COMMAND_LENGTH];
extern int history_count;
extern int history_position;

void kernel_main(void);
void process_command(void);
void add_to_history(const char* command);
void navigate_history(int direction);
void handle_tab_completion(void);

#endif