#ifndef __DEBUG_H__
#define __DEBUG_H__

void debugMagicBreakpoint();

#include "isr.h"
void printISRParam(const isrp_t* p);

#endif
