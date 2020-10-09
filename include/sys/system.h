#ifndef __SYSTEM_H__
#define __SYSTEM_H__

// #include "types.h"
#include <common/types.h>

// hardware io

// read from port
uint8_t inportb(uint16_t port);

// write to port
void outportb(uint16_t port, uint8_t val);



#endif