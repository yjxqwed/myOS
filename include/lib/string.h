#ifndef __STRING_H__
#define __STRING_H__


// #include "types.h"
#include <common/types.h>

#define INT32LEN 12
#define UINT32LEN 13

// signed number to decimal
char* itos(int32_t number, char out[INT32LEN]);

// unsigned number to hexadecimal
char* uitosh(uint32_t number, char out[UINT32LEN]);

// memory set
uint8_t* memset(uint8_t* mem, uint8_t val, uint32_t size);
// memory set word
uint16_t* memsetw(uint16_t* mem, uint16_t val, uint32_t size);

// length of a null terminated string
uint32_t strlen(const char* str);

// Unsafe. Make sure src is null terminated.
char* strcpy(const char* src, char* dest);
// dest is not guaranteed to be null terminated
char* strncpy(const char* src, char* dest, uint32_t n);

#endif