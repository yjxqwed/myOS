// Reference: https://moss.cs.iit.edu/cs351/slides/slides-malloc.pdf

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define ALIGNMENT 8  // must be a power of 2
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))  // header size

void *heap_start = NULL;

typedef struct free_blk_header {
    size_t size;
    struct free_blk_header *next;
    struct free_blk_header *prev;
} free_blk_header_t;

#define MIN_BLK_SIZE ALIGN(sizeof(free_blk_header_t))

// init heap with a permanent (circular) free list head
static void init_heap() {
    if (heap_start != NULL) {
        return;
    }
    free_blk_header_t *bp = heap_start = sbrk(0);
    sbrk(MIN_BLK_SIZE);
    bp->size = 0;
    bp->next = bp;
    bp->prev = bp;
}

static void *find_fit(size_t length) {
    free_blk_header_t *bp = heap_start;
    for (bp = bp->next; bp != heap_start; bp = bp->next) {
        // find first fit
        if (bp->size >= length) {
            // remove from free list and return
            bp->next->prev = bp->prev;
            bp->prev->next = bp->next;
            return bp;
        }
    }
    return NULL;
}

void *malloc(size_t size) {
    // init_heap stuff from before goes here
    if (heap_start == NULL) {
        init_heap();
    }
    int blk_size = ALIGN(size + SIZE_T_SIZE);
    blk_size = (blk_size < MIN_BLK_SIZE) ? MIN_BLK_SIZE : blk_size;
    size_t *header = find_fit(blk_size);
    if (header) {
        *header = *header | 1;
        // FIXME: split if possible
    } else {
        header = sbrk(blk_size);
        *header = blk_size | 1;
    }
    return (char *)header + SIZE_T_SIZE;
}

void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    free_blk_header_t *header = (free_blk_header_t *)((char *)ptr - SIZE_T_SIZE);
    free_blk_header_t *free_list_head = (free_blk_header_t *)heap_start;
    // add freed block to free list after head
    header->size = *(size_t *)header & ~1L;
    header->next = free_list_head->next;
    header->prev = free_list_head;
    free_list_head->next = free_list_head->next->prev = header;
    // FIXME: coalesce! (requires adding footers, too)
    // FIXME: return memory to OS if possible;
}

void print_free_blks() {
    if (heap_start == NULL) {
        return;
    }
    free_blk_header_t *bp = heap_start;
    printf("free_blks: ");
    for (bp = bp->next; bp != heap_start; bp = bp->next) {
        printf("{size=%d}->", bp->size);
    }
    printf("\n");
}
