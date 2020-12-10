#ifndef __PIT_H__
#define __PIT_H__

/**
 * 8253 Programmable Interval Timer (pit)
 */

#include <common/types.h>

void timer_init(uint32_t hz);

#endif