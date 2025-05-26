#include "commands.h"
#include "vga.h"
#include "filesystem.h"
#include "string.h"
#include "keyboard.h"
#include "constants.h"


void resolve_path(const char* path, char* full_path) {
    if (path[0] == '\\') {
        strcpy(full_path, path);
    } else {
        if (strcmp(fs_current_dir, "\\") == 0) {
            strcpy(full_path, "\\");
            strcat(full_path, path);
        } else {
            strcpy(full_path, fs_current_dir);
            strcat(full_path, "\\");
            strcat(full_path, path);
        }
    }
}

void cmd_version(void) {
    vga_print("OSteoporosis [Version ");
    vga_print(VERSION);
    vga_println("]");
    vga_print(COPYRIGHT);
    vga_print(". ");
    vga_println(LICENSE);
}

void cmd_help(void) {
    vga_println("Available commands:");
    vga_println("CAT       - Displays the contents of a file");
    vga_println("CD        - Changes the current directory");
    vga_println("CLS       - Clears the screen");
    vga_println("COLORTEST - Displays a color test");
    vga_println("COPY      - Copies a file");
    vga_println("DEL       - Deletes a file");
    vga_println("DIR       - Lists files and directories");
    vga_println("ECHO      - Displays messages or toggles command echoing");
    vga_println("HELP      - Shows this help message");
    vga_println("MKDIR     - Creates a directory");
    vga_println("MOVE      - Moves a file");
    vga_println("REN       - Renames a file");
    vga_println("RM        - Removes a file (alias for DEL)");
    vga_println("RMDIR     - Removes a directory");
    vga_println("TOUCH     - Creates an empty file");
    vga_println("VER       - Shows version information");
}

void cmd_dir_path(const char* path) {
    char target_path[FS_MAX_FILENAME];
    int is_root;
    
    if (path && path[0] != '\0') {
        resolve_path(path, target_path);
        
        fs_file_t* dir = fs_find(target_path);
        if (!dir) {
            vga_print("Directory not found: ");
            vga_println(path);
            return;
        }
        
        if (dir->type != FS_DIRECTORY) {
            vga_println("Not a directory");
            return;
        }
        
        is_root = (strcmp(target_path, "\\") == 0);
    } else {
        strcpy(target_path, fs_current_dir);
        is_root = (strcmp(fs_current_dir, "\\") == 0);
    }
    
    vga_print(" Directory of C:");
    vga_println(target_path);
    vga_println("");
    
    int file_count = 0;
    int dir_count = 0;
    unsigned int total_size = 0;
    int target_path_len = strlen(target_path);
    
    if (!is_root) {
        vga_print("<DIR>          ");
        vga_println("..");
        dir_count++;
    }
    
    for (int i = 0; i < fs_file_count; i++) {
        if (strcmp(fs_files[i].name, "\\") == 0) {
            continue;
        }
        
        int file_path_len = strlen(fs_files[i].name);
        int is_in_target_dir = 0;
        
        if (is_root) {
            int backslash_count = 0;
            for (int j = 0; j < file_path_len; j++) {
                if (fs_files[i].name[j] == '\\') {
                    backslash_count++;
                }
            }
            
            is_in_target_dir = (backslash_count == 1 && fs_files[i].name[0] == '\\');
        } else {
            if (file_path_len > target_path_len && 
                strncmp(fs_files[i].name, target_path, target_path_len) == 0 &&
                fs_files[i].name[target_path_len] == '\\') {
                
                int additional_backslashes = 0;
                for (int j = target_path_len + 1; j < file_path_len; j++) {
                    if (fs_files[i].name[j] == '\\') {
                        additional_backslashes++;
                    }
                }
                
                is_in_target_dir = (additional_backslashes == 0);
            }
        }
        
        if (!is_in_target_dir) {
            continue;
        }
        
        char name_only[FS_MAX_FILENAME];
        int name_start;
        
        if (is_root) {
            name_start = 1;
        } else {
            name_start = target_path_len + 1;
        }
        
        strcpy(name_only, &fs_files[i].name[name_start]);
        
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

void cmd_dir(void) {
    cmd_dir_path("");
}

void cmd_type(const char* filename) {
    char full_path[FS_MAX_FILENAME];
    resolve_path(filename, full_path);
    
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
    char source_full_path[FS_MAX_FILENAME];
    resolve_path(source, source_full_path);
    
    char dest_full_path[FS_MAX_FILENAME];
    if (strcmp(dest, "..") == 0) {
        char parent_dir[FS_MAX_FILENAME];
        get_parent_dir(fs_current_dir, parent_dir);
        
        char source_filename[FS_MAX_FILENAME];
        const char* last_slash = strrchr(source_full_path, '\\');
        if (last_slash) {
            strcpy(source_filename, last_slash + 1);
        } else {
            strcpy(source_filename, source_full_path);
        }
        
        if (strcmp(parent_dir, "\\") == 0) {
            strcpy(dest_full_path, "\\");
            strcat(dest_full_path, source_filename);
        } else {
            strcpy(dest_full_path, parent_dir);
            strcat(dest_full_path, "\\");
            strcat(dest_full_path, source_filename);
        }
    } else {
        resolve_path(dest, dest_full_path);
    }
    
    fs_file_t* dest_file = fs_find(dest_full_path);
    if (dest_file && dest_file->type == FS_DIRECTORY) {
        char source_filename[FS_MAX_FILENAME];
        const char* last_slash = strrchr(source_full_path, '\\');
        if (last_slash) {
            strcpy(source_filename, last_slash + 1);
        } else {
            strcpy(source_filename, source_full_path);
        }
        
        char new_dest_path[FS_MAX_FILENAME];
        strcpy(new_dest_path, dest_full_path);
        
        if (dest_full_path[strlen(dest_full_path) - 1] != '\\') {
            strcat(new_dest_path, "\\");
        }
        
        strcat(new_dest_path, source_filename);
        
        if (!fs_copy(source_full_path, new_dest_path)) {
            vga_println("Copy failed");
            return;
        }
    } else {
        if (!fs_copy(source_full_path, dest_full_path)) {
            vga_println("Copy failed");
            return;
        }
    }
    
    vga_println("        1 file(s) copied");
}

void cmd_rename(const char* oldname, const char* newname) {
    if (strcmp(newname, "..") == 0) {
        vga_println("Invalid destination name");
        return;
    }

    char old_full_path[FS_MAX_FILENAME];
    resolve_path(oldname, old_full_path);
    
    char new_full_path[FS_MAX_FILENAME];
    if (strcmp(newname, "..") == 0) {
        char parent_dir[FS_MAX_FILENAME];
        get_parent_dir(fs_current_dir, parent_dir);
        
        char filename[FS_MAX_FILENAME];
        const char* last_slash = strrchr(old_full_path, '\\');
        if (last_slash) {
            strcpy(filename, last_slash + 1);
        } else {
            strcpy(filename, old_full_path);
        }
        
        if (strcmp(parent_dir, "\\") == 0) {
            strcpy(new_full_path, "\\");
            strcat(new_full_path, filename);
        } else {
            strcpy(new_full_path, parent_dir);
            strcat(new_full_path, "\\");
            strcat(new_full_path, filename);
        }
    } else {
        resolve_path(newname, new_full_path);
    }
    
    if (!fs_rename(old_full_path, new_full_path)) {
        vga_println("Rename failed");
        return;
    }
}

void cmd_move(const char* source, const char* dest) {
    char source_full_path[FS_MAX_FILENAME];
    resolve_path(source, source_full_path);
    
    char dest_full_path[FS_MAX_FILENAME];
    if (strcmp(dest, "..") == 0) {
        char parent_dir[FS_MAX_FILENAME];
        get_parent_dir(fs_current_dir, parent_dir);
        
        if (parent_dir[0] == '\0') {
            strcpy(dest_full_path, "\\");
        } else {
            strcpy(dest_full_path, parent_dir);
        }
    } else {
        resolve_path(dest, dest_full_path);
    }
    
    fs_file_t* dest_file = fs_find(dest_full_path);
    if (dest_file && dest_file->type == FS_DIRECTORY) {
        char source_filename[FS_MAX_FILENAME];
        const char* last_slash = strrchr(source_full_path, '\\');
        if (last_slash) {
            strcpy(source_filename, last_slash + 1);
        } else {
            strcpy(source_filename, source_full_path);
        }
        
        char new_dest_path[FS_MAX_FILENAME];
        strcpy(new_dest_path, dest_full_path);
        
        if (dest_full_path[strlen(dest_full_path) - 1] != '\\') {
            strcat(new_dest_path, "\\");
        }
        
        strcat(new_dest_path, source_filename);
        
        if (!fs_move(source_full_path, new_dest_path)) {
            vga_println("Move failed");
            return;
        }
    } else {
        if (!fs_move(source_full_path, dest_full_path)) {
            vga_println("Move failed");
            return;
        }
    }
    
    vga_println("        1 file(s) moved");
}

void cmd_del(const char* filename) {
    char full_path[FS_MAX_FILENAME];
    resolve_path(filename, full_path);
    
    fs_file_t* file = fs_find(full_path);
    
    if (!file) {
        vga_print("File not found: ");
        vga_println(filename);
        return;
    }
    
    if (file->type == FS_DIRECTORY) {
        vga_println("Cannot delete directory with DEL. Use RD instead.");
        return;
    }
    
    if (!fs_delete(full_path)) {
        vga_println("Delete failed");
        return;
    }
}

void cmd_mkdir(const char* dirname) {
    char full_path[FS_MAX_FILENAME];
    resolve_path(dirname, full_path);
    
    if (!fs_create_directory(full_path)) {
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

void cmd_rmdir(const char* dirname) {
    char full_path[FS_MAX_FILENAME];
    resolve_path(dirname, full_path);
    
    fs_file_t* dir = fs_find(full_path);
    
    if (!dir) {
        vga_print("Directory not found: ");
        vga_println(dirname);
        return;
    }
    
    // Verify it is a directory
    if (dir->type != FS_DIRECTORY) {
        vga_println("Not a directory");
        return;
    }
    
    // Check if it's the root directory (can't delete root)
    if (strcmp(full_path, "\\") == 0) {
        vga_println("Cannot remove root directory");
        return;
    }
    
    // Check if it's not empty (has files or subdirectories)
    int path_len = strlen(full_path);
    for (int i = 0; i < fs_file_count; i++) {
        // Skip the directory itself
        if (strcmp(fs_files[i].name, full_path) == 0) {
            continue;
        }
        
        // Check if any file or directory is inside this directory
        if (strlen(fs_files[i].name) > path_len && 
            strncmp(fs_files[i].name, full_path, path_len) == 0 && 
            fs_files[i].name[path_len] == '\\') {
            vga_println("Directory not empty");
            return;
        }
    }
    
    // Delete the directory
    if (!fs_delete(full_path)) {
        vga_println("Failed to remove directory");
        return;
    }
    
    // If we deleted the current directory, go up one level
    if (strcmp(fs_current_dir, full_path) == 0) {
        // Find the parent directory
        char parent_dir[FS_MAX_FILENAME];
        get_parent_dir(fs_current_dir, parent_dir);
        
        if (parent_dir[0] != '\0') {
            strcpy(fs_current_dir, parent_dir);
        } else {
            strcpy(fs_current_dir, "\\");  // Go to root
        }
    }
}

void cmd_colortest(void) {
    // Save the original color
    unsigned char original_color = vga_color;
    
    // First row - standard colors
    for (int bg = 0; bg < 8; bg++) {
        vga_set_color(VGA_COLOR_BLACK, bg);
        vga_print("   ");
    }
    vga_println("");
    
    // Second row - bright colors
    for (int bg = 8; bg < 16; bg++) {
        vga_set_color(VGA_COLOR_BLACK, bg);
        vga_print("   ");
    }
    vga_println("");
    
    // Restore the original color
    vga_color = original_color;
}

void cmd_touch(const char* filename) {
    char full_path[FS_MAX_FILENAME];
    resolve_path(filename, full_path);
    
    fs_file_t* existing_file = fs_find(full_path);
    if (existing_file) {
        vga_println("File already exists");
        return;
    }
    
    if (!fs_create_file(full_path, "")) {
        vga_println("Failed to create file");
        return;
    }
    
    vga_print("Created empty file: ");
    vga_println(filename);
}

void cmd_rm(const char* filename) {
    cmd_del(filename);
}