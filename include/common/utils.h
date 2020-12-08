#ifndef __UTILS_H__
#define __UTILS_H__

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// round up of a / b
#define ROUND_UP_DIV(a, b) (((a) + (b) - 1) / (b))
#endif
