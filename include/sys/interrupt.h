#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

// for enable/disable interrupts (by setting/clearing IF)

typedef enum INT_STATUS {
    INTERRUPT_OFF,
    INTERRUPT_ON
} INT_STATUS;

// get current interrupt status
INT_STATUS get_int_status();

// enable interrupt and return the previous status
INT_STATUS enable_int();

// disable interrupt and return the previous status
INT_STATUS disable_int();

#endif