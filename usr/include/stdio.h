#ifndef __USR_STDIO_H__
#define __USR_STDIO_H__

#include <common/types.h>

/**
 * %c: char;
 * %s: string;
 * %d: signed integer;
 * %x: unsigned integer (hex);
 * %X: unsigned integer (full 8 hex digits);
 */

/**
 * @brief print to stdout
 * @return num of chars printed; -1 on failure.
 */
int printf(const char *fmt, ...);

/**
 * @brief print to buf
 * @return num of chars printed; -1 on failure.
 */
int sprintf(char *buf, const char *fmt, ...);

/**
 * @brief getchar
 *        no io buffering
 */
char getchar();
int putchar(char c);

// int getline(char **lineptr, size_t *n);
// int getdelim(char **lineptr, size_t *n, char delim);

#endif
