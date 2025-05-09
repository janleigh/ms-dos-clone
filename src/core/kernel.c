#include "vga.h"
#include "commands.h"
#include "string.h"
#include "keyboard.h"
#include "kernel.h"
#include "filesystem.h"
#include "types.h"

char input_buffer[MAX_COMMAND_LENGTH];
int buffer_position = 0;

char command_history[COMMAND_HISTORY_SIZE][MAX_COMMAND_LENGTH];
int history_count = 0;
int history_position = -1;

// Function prototypes
void kernel_main();
void process_command();
void parse_args(char* input, char* command, char* arg1, char* arg2);
void add_to_history(const char* command);
void navigate_history(int direction);
void handle_tab_completion();

void kernel_main() {
    // Direct VGA buffer access for initial screen setup
    volatile unsigned short* vga_buffer = (volatile unsigned short*)0xB8000;
    
    // Clear screen with direct buffer access
    for (int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = 0x0720; // Space with light gray on black
    }

    // Init VGA driver
    vga_init();
    
    // Init keyboard driver
    keyboard_init();
    
    // Init filesystem driver
    fs_init();
    
    // Print welcome message
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);    
    vga_println("MS-DOS Clone [Version 0.1.0]");
    vga_println("(c) Jan Leigh Munoz and Victor Alexander Ong. Licensed under MIT License.");
    vga_println("");
    
    // Initialize command history
    for (int i = 0; i < COMMAND_HISTORY_SIZE; i++) {
        command_history[i][0] = '\0';
    }
    
    // Show prompt with current directory
    vga_print("C:");
    vga_print(fs_current_dir);
    vga_print("> ");
    
    // Main loop
    while (1) {
        // Poll for keyboard input
        keyboard_handler();
    }
}

void add_to_history(const char* command) {
    // Don't add empty commands or duplicates of the last command
    if (strlen(command) == 0 || (history_count > 0 && strcmp(command, command_history[history_count - 1]) == 0)) {
        return;
    }
    
    // Shift history if it's full
    if (history_count == COMMAND_HISTORY_SIZE) {
        for (int i = 0; i < COMMAND_HISTORY_SIZE - 1; i++) {
            strcpy(command_history[i], command_history[i + 1]);
        }
        history_count--;
    }
    
    // Add the new command
    strcpy(command_history[history_count], command);
    history_count++;
}

void navigate_history(int direction) {
    // No history to navigate
    if (history_count == 0) {
        return;
    }
    
    // Compute new history position
    int new_position = history_position + direction;
    
    // Bounds checking
    if (new_position < -1) {
        new_position = -1;
    } else if (new_position >= history_count) {
        new_position = history_count - 1;
    }
    
    // No change needed
    if (new_position == history_position) {
        return;
    }
    
    // Save current position
    history_position = new_position;
    
    // Clear current input
    clear_input_line();
    
    // If we're at -1, show an empty prompt
    if (history_position == -1) {
        buffer_position = 0;
        input_buffer[0] = '\0';
    } else {
        // Copy from history to input buffer
        strcpy(input_buffer, command_history[history_position]);
        buffer_position = strlen(input_buffer);
        
        // Display the command
        vga_print(input_buffer);
    }
}

// Handle tab completion for filenames
void handle_tab_completion() {
    // Make sure we have something to complete
    if (buffer_position == 0) {
        return;
    }
    
    // Save current input with null termination for string operations
    input_buffer[buffer_position] = '\0';
    
    // Parse to get command and first argument
    char command[32];
    char partial_arg[64];
    char dummy[64];
    parse_args(input_buffer, command, partial_arg, dummy);
    
    // Only try to complete if we have a partial argument and it's for a file-based command
    if (partial_arg[0] != '\0' &&
        (strcmp(command, "type") == 0 || strcmp(command, "cat") == 0 ||
            strcmp(command, "del") == 0 || strcmp(command, "delete") == 0 ||
            strcmp(command, "copy") == 0 || strcmp(command, "cp") == 0 ||
            strcmp(command, "rename") == 0 || strcmp(command, "ren") == 0 ||
            strcmp(command, "move") == 0 || strcmp(command, "mv") == 0)) {
        
        // Try to find matching files
        char* match = NULL;
        int matches = 0;
        
        for (int i = 0; i < fs_file_count; i++) {
            // Skip directories for file operations
            if (fs_files[i].type == FS_DIRECTORY) {
                continue;
            }
            
            // Check if file name starts with the partial argument
            int j = 0;
            while (partial_arg[j] != '\0' && fs_files[i].name[j] != '\0' && 
                (partial_arg[j] == fs_files[i].name[j] || 
                    (partial_arg[j] >= 'a' && partial_arg[j] <= 'z' && 
                        partial_arg[j] - 32 == fs_files[i].name[j]) ||
                    (partial_arg[j] >= 'A' && partial_arg[j] <= 'Z' && 
                        partial_arg[j] + 32 == fs_files[i].name[j]))) {
                j++;
            }
            
            // If we reached the end of the partial argument, we have a match
            if (partial_arg[j] == '\0') {
                match = fs_files[i].name;
                matches++;
            }
        }
        
        // If there's exactly one match, complete it
        if (matches == 1) {
            // Clear the current command line
            clear_input_line();
            
            // Reconstruct the command with the completed filename
            strcpy(input_buffer, command);
            strcat(input_buffer, " ");
            strcat(input_buffer, match);
            buffer_position = strlen(input_buffer);
            
            // Display the updated command
            vga_print(input_buffer);
        }
        // If there are multiple matches, we could show possibilities
        // but I ain't doing that, fuck you
        else if (matches > 1) {
            
        }
    }
    // Directory completion for cd/mkdir
    else if (partial_arg[0] != '\0' &&
            (strcmp(command, "cd") == 0 || strcmp(command, "chdir") == 0 ||
                strcmp(command, "mkdir") == 0 || strcmp(command, "md") == 0)) {
        
        // Try to find matching directories
        char* match = NULL;
        int matches = 0;
        
        for (int i = 0; i < fs_file_count; i++) {
            // Skip files for directory operations
            if (fs_files[i].type != FS_DIRECTORY) {
                continue;
            }
            
            // Check if directory name starts with the partial argument
            int j = 0;
            while (partial_arg[j] != '\0' && fs_files[i].name[j] != '\0' && 
                    (partial_arg[j] == fs_files[i].name[j] || 
                        (partial_arg[j] >= 'a' && partial_arg[j] <= 'z' && 
                            partial_arg[j] - 32 == fs_files[i].name[j]) ||
                        (partial_arg[j] >= 'A' && partial_arg[j] <= 'Z' && 
                            partial_arg[j] + 32 == fs_files[i].name[j]))) {
                j++;
            }
            
            // If we reached the end of the partial argument, we have a match
            if (partial_arg[j] == '\0') {
                match = fs_files[i].name;
                matches++;
            }
        }
        
        // If there's exactly one match, complete it
        if (matches == 1) {
            // Clear the current command line
            clear_input_line();
            
            // Reconstruct the command with the completed directory name
            strcpy(input_buffer, command);
            strcat(input_buffer, " ");
            strcat(input_buffer, match);
            buffer_position = strlen(input_buffer);
            
            // Display the updated command
            vga_print(input_buffer);
        }
    }
}

// Simple argument parser - splits a command line into command and two arguments
void parse_args(char* input, char* command, char* arg1, char* arg2) {
    int i = 0;
    int j = 0;
    
    // First, extract the command
    while (input[i] != ' ' && input[i] != '\0') {
        command[j++] = input[i++];
    }
    command[j] = '\0';
    
    // If we've hit the end of the string, there are no arguments
    if (input[i] == '\0') {
        arg1[0] = '\0';
        arg2[0] = '\0';
        return;
    }
    
    // Skip spaces
    while (input[i] == ' ') {
        i++;
    }
    
    // Extract first argument
    j = 0;
    while (input[i] != ' ' && input[i] != '\0') {
        arg1[j++] = input[i++];
    }
    arg1[j] = '\0';
    
    // If we've hit the end of the string, there is no second argument
    if (input[i] == '\0') {
        arg2[0] = '\0';
        return;
    }
    
    // Skip spaces
    while (input[i] == ' ') {
        i++;
    }
    
    // Extract second argument
    j = 0;
    while (input[i] != ' ' && input[i] != '\0') {
        arg2[j++] = input[i++];
    }
    arg2[j] = '\0';
}

void process_command() {
    // Null-terminate the command
    input_buffer[buffer_position] = '\0';
    vga_println("");
    
    // Check if the command is empty
    if (input_buffer[0] == '\0') {
        // Show prompt and return
        vga_print("C:");
        vga_print(fs_current_dir);
        vga_print("> ");
        return;
    }
    
    // Special handling for the ECHO command which may contain spaces
    if (strncmp(input_buffer, "echo ", 5) == 0 || 
        strncmp(input_buffer, "ECHO ", 5) == 0 || 
        strcmp(input_buffer, "echo") == 0 || 
        strcmp(input_buffer, "ECHO") == 0) {
        
        // If it's just "echo" with no arguments
        if (strlen(input_buffer) == 4 || strlen(input_buffer) == 5) {
            cmd_echo("");
        } else {
            // Pass everything after "echo " as the text argument
            cmd_echo(input_buffer + 5);
        }
        
        // Show prompt and return
        vga_print("C:");
        vga_print(fs_current_dir);
        vga_print("> ");
        return;
    }
    
    // For other commands, use the existing parsing logic
    char command[32];
    char arg1[64];
    char arg2[64];
    parse_args(input_buffer, command, arg1, arg2);
    
    // Convert command to lowercase for case-insensitive comparison
    for (int i = 0; command[i]; i++) {
        if (command[i] >= 'A' && command[i] <= 'Z') {
            command[i] = command[i] + 32;
        }
    }
    
    if (strcmp(command, "version") == 0 || strcmp(command, "ver") == 0) {
        cmd_version();
    } else if (strcmp(command, "cls") == 0 || strcmp(command, "clear") == 0) {
        vga_clear_screen();
    } else if (strcmp(command, "help") == 0) {
        cmd_help();
    } else if (strcmp(command, "dir") == 0 || strcmp(command, "ls") == 0) {
        cmd_dir();
    } else if (strcmp(command, "type") == 0 || strcmp(command, "cat") == 0) {
        if (arg1[0] == '\0') {
            vga_println("Syntax: TYPE <filename>");
        } else {
            cmd_type(arg1);
        }
    } else if (strcmp(command, "copy") == 0 || strcmp(command, "cp") == 0) {
        if (arg1[0] == '\0' || arg2[0] == '\0') {
            vga_println("Syntax: COPY <source> <destination>");
        } else {
            cmd_copy(arg1, arg2);
        }
    } else if (strcmp(command, "rename") == 0 || strcmp(command, "ren") == 0) {
        if (arg1[0] == '\0' || arg2[0] == '\0') {
            vga_println("Syntax: REN <oldname> <newname>");
        } else {
            cmd_rename(arg1, arg2);
        }
    } else if (strcmp(command, "move") == 0 || strcmp(command, "mv") == 0) {
        if (arg1[0] == '\0' || arg2[0] == '\0') {
            vga_println("Syntax: MOVE <source> <destination>");
        } else {
            cmd_move(arg1, arg2);
        }
    } else if (strcmp(command, "delete") == 0 || strcmp(command, "del") == 0) {
        if (arg1[0] == '\0') {
            vga_println("Syntax: DEL <filename>");
        } else {
            cmd_del(arg1);
        }
    } else if (strcmp(command, "mkdir") == 0 || strcmp(command, "md") == 0) {
        if (arg1[0] == '\0') {
            vga_println("Syntax: MKDIR <dirname>");
        } else {
            cmd_mkdir(arg1);
        }
    } else if (strcmp(command, "cd") == 0 || strcmp(command, "chdir") == 0) {
        cmd_cd(arg1);
    } else if (strcmp(command, "color") == 0) {
        cmd_color();
    } else if (strlen(command) > 0) {
        vga_print("Bad command or file name: ");
        vga_println(command);
    }
    
    // Reset buffer and show new prompt with current directory
    buffer_position = 0;
    vga_print("C:");
    vga_print(fs_current_dir);
    vga_print("> ");
}