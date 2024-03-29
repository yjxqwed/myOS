#ifndef __UTILS_H__
#define __UTILS_H__

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// round up of a / b
#define ROUND_UP_DIV(a, b) (((a) + (b) - 1) / (b))
#define ROUND_UP(a, b) (ROUND_UP_DIV(a, b) * b)

#define __attr_packed __attribute__((packed))

#endif
