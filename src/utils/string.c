#include "string.h"

int strlen(const char* str) {
    int len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

char* strcat(char* dest, const char* src) {
    char* dest_end = dest + strlen(dest);
    while (*src) {
        *dest_end++ = *src++;
    }
    *dest_end = '\0';
    return dest;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Skip leading whitespace
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

void itoa(int value, char* str, int base) {
    static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char* buffer = str;
    char* ptr;
    int sign = 0;
    
    // Handle negative numbers only for base 10
    if (base == 10 && value < 0) {
        sign = 1;
        value = -value;
    }
    
    // Generate digits in reverse order
    ptr = buffer;
    do {
        *ptr++ = digits[value % base];
        value /= base;
    } while (value);
    
    // Add sign if needed
    if (sign) {
        *ptr++ = '-';
    }
    
    // Terminate the string
    *ptr = '\0';
    
    // Reverse the string
    ptr--;
    while (buffer < ptr) {
        char temp = *buffer;
        *buffer = *ptr;
        *ptr = temp;
        buffer++;
        ptr--;
    }
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        --n;
    }
    
    if (n == 0) {
        return 0;
    }
    
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strchr(const char* s, int c) {
    while (*s != '\0') {
        if (*s == c) {
            return (char*)s;  // Cast away const for compatibility
        }
        s++;
    }
    
    // Also check for the null terminator if c is 0
    if (c == 0) {
        return (char*)s;
    }
    
    return NULL;  // Not found
}

size_t strspn(const char* str, const char* accept) {
    const char* s = str;
    
    while (*s) {
        const char* a = accept;
        int found = 0;
        
        while (*a) {
            if (*s == *a) {
                found = 1;
                break;
            }
            a++;
        }
        
        if (!found) {
            break;
        }
        
        s++;
    }
    
    return s - str;
}

char* strpbrk(const char* str, const char* accept) {
    while (*str) {
        const char* a = accept;
        
        while (*a) {
            if (*str == *a) {
                return (char*)str;
            }
            a++;
        }
        
        str++;
    }
    
    return NULL;
}

void strtok(char* str, const char* delim, char** saveptr, char** token) {
    char* start;
    
    // If str is NULL, continue from saveptr
    if (str == NULL) {
        str = *saveptr;
    }
    
    // Skip leading delimiters
    str += strspn(str, delim);
    
    // If we've reached the end, return NULL
    if (*str == '\0') {
        *saveptr = str;
        *token = NULL;
        return;
    }
    
    // Find the end of the token
    start = str;
    str = strpbrk(str, delim);
    
    if (str == NULL) {
        // This token finishes the string
        *saveptr = strchr(start, '\0');
    } else {
        // Terminate the token and make saveptr point past it
        *str = '\0';
        *saveptr = str + 1;
    }
    
    *token = start;
}