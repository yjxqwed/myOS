// isr.h/.c are for the (i)nterrupt (s)ervice (r)outines

#ifndef __ISR_H__
#define __ISR_H__s

struct InterruptServiceRoutineParam {

};

void setISRs();

typedef struct InterruptServiceRoutineParam isrp_t;
#endif