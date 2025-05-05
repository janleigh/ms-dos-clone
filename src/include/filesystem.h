#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define FS_MAX_FILES 64
#define FS_MAX_FILENAME 32
#define FS_MAX_CONTENT 4096  // 4kb max size

// File type flags
#define FS_FILE 0x01
#define FS_DIRECTORY 0x02

typedef struct {
    char name[FS_MAX_FILENAME];
    unsigned char type;        // File or directory
    unsigned int size;         // Size of file content
    char content[FS_MAX_CONTENT];
} fs_file_t;

extern fs_file_t fs_files[FS_MAX_FILES];
extern int fs_file_count;
extern char fs_current_dir[FS_MAX_FILENAME];

void fs_init(void);
int fs_create_file(const char* name, const char* content);
int fs_create_directory(const char* name);
int fs_delete(const char* name);
int fs_rename(const char* oldname, const char* newname);
int fs_copy(const char* source, const char* dest);
int fs_move(const char* source, const char* dest);
fs_file_t* fs_find(const char* name);
void fs_list_directory(void);

#endif