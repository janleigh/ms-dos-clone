#include "filesystem.h"
#include "string.h"
#include "constants.h"

fs_file_t fs_files[FS_MAX_FILES];
int fs_file_count = 0;
char fs_current_dir[FS_MAX_FILENAME] = "\\";

void fs_init(void) {
    fs_file_count = 0;
    strcpy(fs_current_dir, "\\");
    
    fs_create_directory("\\");
    fs_create_directory("\\SYSTEM");
    fs_create_directory("\\DOCUMENTS");
    fs_create_directory("\\PICTURES");
    fs_create_directory("\\MUSIC");
    fs_create_directory("\\VIDEOS");
    
    fs_create_file("\\README.TXT", DEFAULT_README_CONTENT);
    fs_create_file("\\VERSION.TXT", DEFAULT_VERSION_CONTENT);
    fs_create_file("\\LICENSE.TXT", DEFAULT_LICENSE_CONTENT);
}

// New helper function to find parent directory path
void get_parent_dir(const char* path, char* parent) {
    strcpy(parent, path);
    
    // Find the last backslash
    int last_slash = -1;
    for (int i = strlen(parent) - 1; i >= 0; i--) {
        if (parent[i] == '\\') {
            last_slash = i;
            break;
        }
    }
    
    // If found, truncate after it, otherwise clear the parent
    if (last_slash >= 0) {
        if (last_slash == 0) {
            parent[1] = '\0';  // For root directory "\"
        } else {
            parent[last_slash] = '\0';
        }
    } else {
        parent[0] = '\0';
    }
}

int fs_create_file(const char* name, const char* content) {
    // Check if we have space for more files
    if (fs_file_count >= FS_MAX_FILES) {
        return 0;
    }
    
    // Check if file already exists
    if (fs_find(name) != 0) {
        return 0;
    }
    
    // Create the new file
    strcpy(fs_files[fs_file_count].name, name);
    fs_files[fs_file_count].type = FS_FILE;
    fs_files[fs_file_count].size = strlen(content);
    strcpy(fs_files[fs_file_count].content, content);
    fs_file_count++;
    
    return 1;
}

int fs_create_directory(const char* name) {
    // Check if we have space for more files
    if (fs_file_count >= FS_MAX_FILES) {
        return 0;
    }
    
    // Check if directory already exists
    if (fs_find(name) != 0) {
        return 0;
    }
    
    // If it's not the root directory, check if parent directory exists
    if (strcmp(name, "\\") != 0) {
        char parent_dir[FS_MAX_FILENAME];
        get_parent_dir(name, parent_dir);
        
        // If parent_dir is empty, that's an error
        if (parent_dir[0] == '\0') {
            return 0;
        }
        
        // Find the parent directory
        fs_file_t* parent = fs_find(parent_dir);
        if (!parent || parent->type != FS_DIRECTORY) {
            return 0;  // Parent directory doesn't exist
        }
    }
    
    strcpy(fs_files[fs_file_count].name, name);
    fs_files[fs_file_count].type = FS_DIRECTORY;
    fs_files[fs_file_count].size = 0;
    fs_files[fs_file_count].content[0] = '\0';
    fs_file_count++;
    
    return 1;
}

int fs_delete(const char* name) {
    // Find the file or directory
    int i;
    for (i = 0; i < fs_file_count; i++) {
        if (strcmp(fs_files[i].name, name) == 0) {
            // Found the file/directory to delete
            // Move all items after this one up to fill the gap
            for (int j = i; j < fs_file_count - 1; j++) {
                strcpy(fs_files[j].name, fs_files[j+1].name);
                fs_files[j].type = fs_files[j+1].type;
                fs_files[j].size = fs_files[j+1].size;
                strcpy(fs_files[j].content, fs_files[j+1].content);
            }
            fs_file_count--;
            return 1;
        }
    }
    
    return 0;
}

int fs_rename(const char* oldname, const char* newname) {
    // Check if the new name already exists
    if (fs_find(newname) != 0) {
        return 0; 
    }
    
    // Find the file to rename
    fs_file_t* file = fs_find(oldname);
    if (!file) {
        return 0;
    }
    
    strcpy(file->name, newname);
    return 1;
}

int fs_copy(const char* source, const char* dest) {
    // Find the source file
    fs_file_t* src_file = fs_find(source);
    if (!src_file || src_file->type != FS_FILE) {
        return 0;
    }
    
    return fs_create_file(dest, src_file->content);
}

int fs_move(const char* source, const char* dest) {
    // First copy the file
    if (!fs_copy(source, dest)) {
        return 0;
    }
    
    return fs_delete(source);
}

fs_file_t* fs_find(const char* name) {
    for (int i = 0; i < fs_file_count; i++) {
        if (strcmp(fs_files[i].name, name) == 0) {
            return &fs_files[i];
        }
    }
    return 0;
}

void fs_list_directory(void) {
    for (int i = 0; i < fs_file_count; i++) {
        // Skip the root directory entry
        if (strcmp(fs_files[i].name, "\\") == 0) {
            continue;
        }
        
        // TODO: Implement this.
        // Print the file or directory name and details
        if (fs_files[i].type == FS_DIRECTORY) {
            // It's a directory
        } else {
            // It's a file
        }
    }
}