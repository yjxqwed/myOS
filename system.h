#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "types.h"

// memory set
uint8_t* memset(uint8_t* mem, uint8_t val, uint32_t size);
uint16_t* memsetw(uint16_t* mem, uint16_t val, uint32_t size);

// string
uint32_t strlen(char* str);

// hardware io

// read from port
uint8_t inportb(uint16_t port);

// write to port
void outportb(uint16_t port, uint8_t val);

#endif