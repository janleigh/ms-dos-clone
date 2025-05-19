#include "commands.h"
#include "vga.h"
#include "filesystem.h"
#include "string.h"
#include "keyboard.h"
#include "constants.h"

void cmd_version(void) {
    vga_print("MS-DOS Clone [Version ");
    vga_print(VERSION);
    vga_println("]");
    vga_print(COPYRIGHT);
    vga_print(". ");
    vga_println(LICENSE);
}

void cmd_help(void) {
    vga_println("Available commands:");
    vga_println("CD        - Changes the current directory");
    vga_println("CLS       - Clears the screen");
    vga_println("COLOR     - Displays a color test");
    vga_println("COPY      - Copies a file");
    vga_println("DEL       - Deletes a file");
    vga_println("DIR       - Lists files and directories");
    vga_println("ECHO      - Displays messages or toggles command echoing");
    vga_println("HELP      - Shows this help message");
    vga_println("MKDIR     - Creates a directory");
    vga_println("MOVE      - Moves a file");
    vga_println("REN       - Renames a file");
    vga_println("TYPE      - Displays the contents of a file");
    vga_println("VER       - Shows version information");
}

void cmd_dir(void) {
    vga_print(" Directory of C:");
    vga_println(fs_current_dir);
    vga_println("");
    
    int file_count = 0;
    int dir_count = 0;
    unsigned int total_size = 0;
    int current_path_len = strlen(fs_current_dir);
    int is_root = (strcmp(fs_current_dir, "\\") == 0);
    
    // Always show parent directory if not in root
    if (!is_root) {
        vga_print("<DIR>          ");
        vga_println("..");
        dir_count++;
    }
    
    for (int i = 0; i < fs_file_count; i++) {
        // Skip files that are not in the current directory
        if (strcmp(fs_files[i].name, "\\") == 0) {
            // Skip root entry
            continue;
        }
        
        // Check if this file/directory is directly in the current directory
        int file_path_len = strlen(fs_files[i].name);
        int is_in_current_dir = 0;
        
        if (is_root) {
            // For root directory, check if there's only one backslash at position 0
            int backslash_count = 0;
            for (int j = 0; j < file_path_len; j++) {
                if (fs_files[i].name[j] == '\\') {
                    backslash_count++;
                }
            }
            
            is_in_current_dir = (backslash_count == 1 && fs_files[i].name[0] == '\\');
        } else {
            // Check if file's path starts with current path + backslash
            // and doesn't have any more backslashes after that
            if (file_path_len > current_path_len && 
                strncmp(fs_files[i].name, fs_current_dir, current_path_len) == 0 &&
                fs_files[i].name[current_path_len] == '\\') {
                
                // Check for additional backslashes
                int additional_backslashes = 0;
                for (int j = current_path_len + 1; j < file_path_len; j++) {
                    if (fs_files[i].name[j] == '\\') {
                        additional_backslashes++;
                    }
                }
                
                is_in_current_dir = (additional_backslashes == 0);
            }
        }
        
        if (!is_in_current_dir) {
            continue;
        }
        
        // Extract just the filename/dirname from the full path
        char name_only[FS_MAX_FILENAME];
        int name_start;
        
        if (is_root) {
            name_start = 1;
        } else {
            name_start = current_path_len + 1;
        }
        
        strcpy(name_only, &fs_files[i].name[name_start]);
        
        // Print the file or directory details
        if (fs_files[i].type == FS_DIRECTORY) {
            vga_print("<DIR>          ");
            vga_println(name_only);
            dir_count++;
        } else {
            char size_str[16];
            itoa(fs_files[i].size, size_str, 10);
            
            int pad = 14 - strlen(size_str);
            for (int j = 0; j < pad; j++) {
                vga_putchar(' ');
            }
            
            vga_print(size_str);
            vga_print(" ");
            vga_println(name_only);
            
            file_count++;
            total_size += fs_files[i].size;
        }
    }
    
    vga_println("");
    
    char file_count_str[16];
    char dir_count_str[16];
    char total_size_str[16];
    
    itoa(file_count, file_count_str, 10);
    itoa(dir_count, dir_count_str, 10);
    itoa(total_size, total_size_str, 10);
    
    vga_print(file_count_str);
    vga_print(" File(s)    ");
    vga_print(total_size_str);
    vga_println(" bytes");
    
    vga_print(dir_count_str);
    vga_println(" Dir(s)");
}

void cmd_type(const char* filename) {
    // Create a full path if filename is relative
    char full_path[FS_MAX_FILENAME];
    
    // Check if it's an absolute path
    if (filename[0] == '\\') {
        strcpy(full_path, filename);
    } else {
        if (strcmp(fs_current_dir, "\\") == 0) {
            strcpy(full_path, "\\");
            strcat(full_path, filename);
        } else {
            strcpy(full_path, fs_current_dir);
            strcat(full_path, "\\");
            strcat(full_path, filename);
        }
    }
    
    // Find using the full path
    fs_file_t* file = fs_find(full_path);
    
    if (!file) {
        vga_print("File not found: ");
        vga_println(full_path);
        return;
    }
    
    if (file->type == FS_DIRECTORY) {
        vga_println("Cannot display directory contents");
        return;
    }
    
    vga_println(file->content);
}

void cmd_copy(const char* source, const char* dest) {
    if (!fs_copy(source, dest)) {
        vga_println("Copy failed");
        return;
    }
    
    vga_println("        1 file(s) copied");
}

void cmd_rename(const char* oldname, const char* newname) {
    if (!fs_rename(oldname, newname)) {
        vga_println("Rename failed");
        return;
    }
}

void cmd_move(const char* source, const char* dest) {
    if (!fs_move(source, dest)) {
        vga_println("Move failed");
        return;
    }
    
    vga_println("        1 file(s) moved");
}

void cmd_del(const char* filename) {
    fs_file_t* file = fs_find(filename);
    
    if (!file) {
        vga_print("File not found: ");
        vga_println(filename);
        return;
    }
    
    if (file->type == FS_DIRECTORY) {
        vga_println("Cannot delete directory with DEL. Use RD instead.");
        return;
    }
    
    if (!fs_delete(filename)) {
        vga_println("Delete failed");
        return;
    }
}

void cmd_mkdir(const char* dirname) {
    if (!fs_create_directory(dirname)) {
        vga_println("Failed to create directory");
        return;
    }
}

void cmd_cd(const char* dirname) {
    // Case 1: If no directory is specified, just print the current directory
    if (dirname[0] == '\0') {
        vga_print("C:");
        vga_println(fs_current_dir);
        return;
    }
    
    // Case 2: Go to root directory
    if (strcmp(dirname, "\\") == 0) {
        strcpy(fs_current_dir, "\\");
        return;
    }
    
    // Case 3: Go up one directory
    if (strcmp(dirname, "..") == 0) {
        // If already at root, do nothing
        if (strcmp(fs_current_dir, "\\") == 0) {
            return;
        }
        
        // Find the last backslash in the current path
        int last_slash = -1;
        for (int i = strlen(fs_current_dir) - 1; i >= 0; i--) {
            if (fs_current_dir[i] == '\\') {
                last_slash = i;
                break;
            }
        }
        
        // If we found a backslash that's not at position 0, truncate after it
        if (last_slash > 0) {
            fs_current_dir[last_slash] = '\0';
        } else {
            // If the backslash is at position 0, we're going to root
            strcpy(fs_current_dir, "\\");
        }
        return;
    }
    
    // Case 4: Handle absolute and relative paths
    char target_path[FS_MAX_FILENAME];
    
    // Check if it's an absolute path (starts with backslash)
    if (dirname[0] == '\\') {
        strcpy(target_path, dirname);
    } else {
        // It's a relative path, so prepend the current directory
        // Make sure we handle the root directory special case
        if (strcmp(fs_current_dir, "\\") == 0) {
            strcpy(target_path, "\\");
            strcat(target_path, dirname);
        } else {
            strcpy(target_path, fs_current_dir);
            strcat(target_path, "\\");
            strcat(target_path, dirname);
        }
    }
    
    // Now check if the directory exists
    fs_file_t* dir = fs_find(target_path);
    
    if (!dir) {
        vga_print("Directory not found: ");
        vga_println(dirname);
        return;
    }
    
    if (dir->type != FS_DIRECTORY) {
        vga_println("Not a directory");
        return;
    }
    
    // Update the current directory
    strcpy(fs_current_dir, target_path);
}

void cmd_color(void) {
    // Save the original color
    unsigned char original_color = vga_color;
    
    vga_println("VGA Color Test:");
    vga_println("---------------");
    vga_println("");
    
    // Display text samples for all 16 foreground colors on black background
    vga_println("Foreground colors (on black background):");
    for (int fg = 0; fg < 16; fg++) {
        vga_set_color(fg, VGA_COLOR_BLACK);
        vga_print("Color ");
        
        char num[3];
        itoa(fg, num, 10);
        vga_print(num);
        vga_print(": ");
        
        vga_println("This is a color test sample text.");
    }
    
    vga_println("");
    
    // Display text samples for all 16 background colors with white text
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_println("Background colors (with white text):");
    for (int bg = 0; bg < 16; bg++) {
        vga_set_color(VGA_COLOR_WHITE, bg);
        vga_print("Color ");
        
        char num[3];
        itoa(bg, num, 10);
        vga_print(num);
        vga_print(": ");
        
        vga_println("This is a color test sample text.");
    }
    
    vga_println("");
    
    // Color name table
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_println("Color index reference:");
    vga_println("0 = BLACK        8 = DARK_GREY");
    vga_println("1 = BLUE         9 = LIGHT_BLUE");
    vga_println("2 = GREEN       10 = LIGHT_GREEN");
    vga_println("3 = CYAN        11 = LIGHT_CYAN");
    vga_println("4 = RED         12 = LIGHT_RED");
    vga_println("5 = MAGENTA     13 = LIGHT_MAGENTA");
    vga_println("6 = BROWN       14 = LIGHT_BROWN");
    vga_println("7 = LIGHT_GREY  15 = WHITE");
    
    // Restore the original color
    vga_color = original_color;
}

void cmd_echo(const char* text) {
    // If no text or empty string, just print a blank line
    if (!text || text[0] == '\0') {
        vga_println("");
        return;
    }
    
    // Handle ECHO ON and ECHO OFF (for batch files)
    if (strcmp(text, "ON") == 0) {
        vga_println("ECHO is on");
        return;
    } else if (strcmp(text, "OFF") == 0) {
        vga_println("ECHO is off");
        return;
    }
    
    // Handle special sequences
    int i = 0;
    while (text[i] != '\0') {
        if (text[i] == '\\' && text[i+1] != '\0') {
            switch (text[i+1]) {
                case 'n':
                    vga_putchar('\n');
                    break;
                case 't':
                    vga_putchar('\t');
                    break;
                case 'r':
                    vga_putchar('\r');
                    break;
                case '\\':
                    vga_putchar('\\');
                    break;
                default:
                    vga_putchar(text[i]);
                    vga_putchar(text[i+1]);
                    break;
            }
            // Skip the second character of the escape sequence
            i += 2;
        } else {
            vga_putchar(text[i]);
            i++;
        }
    }
    
    vga_putchar('\n');
}