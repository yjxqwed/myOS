#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned int size_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;

typedef uint32_t uintptr_t;
typedef int32_t intptr_t;

typedef float float32_t;
typedef double float64_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define False (0)
#define True (!False)

typedef int bool_t;
typedef int off_t;

// a easy pair for array
// __pair(uint32_t, uint32_t) pairs[13];
#define __pair(type1, type2) \
struct { \
    type1 first; \
    type2 second; \
}

#endif
