#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_version(void);
void cmd_help(void);
void cmd_dir(void);
void cmd_type(const char* filename);
void cmd_copy(const char* source, const char* dest);
void cmd_rename(const char* oldname, const char* newname);
void cmd_move(const char* source, const char* dest);
void cmd_del(const char* filename);
void cmd_mkdir(const char* dirname);
void cmd_cd(const char* dirname);
void cmd_color(void);
void cmd_echo(const char* text);

#endif