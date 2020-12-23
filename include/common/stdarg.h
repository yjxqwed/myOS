#ifndef __STDARG_H__
#define __STDARG_H__

/**
 * 
 * @file common/stdarg.h
 * @brief for variable arg list
 * 
 */


#define args_list char *

#define _arg_stack_size(type) ( \
    ((sizeof(type) - 1) / sizeof(int32_t) + 1) * sizeof(int32_t) \
)

#define args_start(ap, fmt) do { \
    ap = (char *)((unsigned int)(&fmt) + _arg_stack_size(&fmt)); \
} while (0);

#define args_end(ap) do { \
    ap = NULL; \
} while (0);

#define args_next(ap, type) ( \
    ((type *)(ap += _arg_stack_size(type)))[-1] \
)

#endif