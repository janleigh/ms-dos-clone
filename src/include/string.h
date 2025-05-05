#ifndef STRING_H
#define STRING_H

#include "types.h"

int strlen(const char* str);
void strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
void itoa(int value, char* str, int base);
char* strchr(const char* s, int c);
void strtok(char* str, const char* delim, char** saveptr, char** token);

#endif