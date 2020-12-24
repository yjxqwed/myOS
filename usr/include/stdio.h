#ifndef __USR_STDIO_H__
#define __USR_STDIO_H__

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

#endif
