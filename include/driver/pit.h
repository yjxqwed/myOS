#ifndef __PIT_H__
#define __PIT_H__

#include <sys/isr.h>
#include <common/types.h>
// Init PIT
void timer_install(uint32_t hz);

// PIT interrupt handler
void do_timer(isrp_t *p);

#endif