#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <common/types.h>

typedef struct CircularBuffer {
    // the actual buffer
    void *buff;
    // size of the buffer
    uint32_t buff_size;
    // current number fo buffered objects
    uint32_t curr_num;
    // buffer head pointer
    uint32_t head;
    // buffer tail pointer
    uint32_t tail;
} cir_buff_t;




#endif
