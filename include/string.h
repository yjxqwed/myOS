#ifndef __STRING_H__
#define __STRING_H__


#include "types.h"

#define INT32LEN 12
#define UINT32LEN 13

// signed number to decimal
char* itos(int32_t number, char out[INT32LEN]);

// unsigned number to hexadecimal
char* uitosh(uint32_t number, char out[13]);

// memory set
uint8_t* memset(uint8_t* mem, uint8_t val, uint32_t size);
uint16_t* memsetw(uint16_t* mem, uint16_t val, uint32_t size);

// string
uint32_t strlen(const char* str);

char* strcpy(const char* src, char* dest);
char* strncpy(const char* src, char* dest, uint32_t n);

#endif