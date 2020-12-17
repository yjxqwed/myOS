#include <mm/kvmm.h>
#include <common/debug.h>
#include <arch/x86.h>
#include <mm/pmem.h>
#include <kprintf.h>
#include <list.h>
#include <common/utils.h>
#include <thread/sync.h>

void *k_get_free_pages(uint32_t pgcnt, uint32_t gfp_flags) {
    ppage_t *fp = pages_alloc(pgcnt, gfp_flags);
    void *kva = NULL;
    if (fp) {
        for (int i = 0; i < pgcnt; i++) {
            page_incref(fp + i);
        }
        kva = page2kva(fp);
    }
    return kva;
}


void k_free_pages(void *kva, uint32_t pgcnt) {
    if (kva == NULL) {
        return;
    } else if (!(__valid_kva(kva) && __page_aligned(kva))) {
        PANIC("bad pointer for k_free_pages");
    }
    for (int i = 0; i < pgcnt; i++) {
        ppage_t *p = kva2page(
            (void *)((uintptr_t)kva + i * PAGE_SIZE)
        );
#ifdef KDEBUG
        mutex_lock(&(p->page_lock));
        ASSERT(p->num_ref == 1);
        mutex_unlock(&(p->page_lock));
#endif
        page_decref(p);
    }
}


/**
 *  Below are kmalloc/kfree
 */

// there are 16 kinds of blocks 32+B each -> 1024+B each
#define MIN_BLK_SIZE    32
#define NR_MEM_BLK_DESC 6


/**
 *  +---------+          \
 *  |tag      |          |    mem_blk_t: header of the actuall memory block.
 *  |magic    |           >   For kfree to check whether the pointer passed in
 *  |data_addr| ---+     |    is a valid one.
 *  +---------+    |     /
 *  |         | <--+------->  start addr of the allocated memory block,
 *  |         |               16B aligned (must be 0xABCDEFG0)
 */

typedef struct MemoryBlock {
    // tag in free_list
    list_node_t tag;
    // for free to check
    uint32_t magic;
    uintptr_t data_addr;
} mem_blk_t;

static void mem_blk_init(mem_blk_t *mb) {
    mb->tag.prev = mb->tag.prev = NULL;
    mb->magic = 0x19971015;
    mb->data_addr = (uintptr_t)(mb + 1);
}

typedef struct MemoryBlockDescriptor {
    // size of blocks in this arena (32B, 64B, etc)
    uint32_t block_size;
    // number of blocks in this arena
    uint32_t nr_blocks_per_arena;
    // list of free blocks
    list_t free_list;

    // for arena_get_blk use
    // address of the first mem_blk_t in an arena
    uintptr_t first;
    // num of bytes between two adjacent mem_blk_ts
    uint32_t delta;

} mem_blk_desc_t;

// array of mem blk descriptors
static mem_blk_desc_t k_mem_blk_descs[NR_MEM_BLK_DESC];

typedef struct Arena {
    // descriptor of blocks in this arena
    // each arena only has one kind of block
    mem_blk_desc_t *desc;
    // if (large), this arena occupies more than one page
    bool_t large;
    // if (large) cnt is page count
    // else cnt is block count
    uint32_t cnt;

    mutex_t arena_lock;
} arena_t;

#define MEM_BLK_ALIGNMENT 16

void block_desc_init(mem_blk_desc_t descs[NR_MEM_BLK_DESC]) {
    uint32_t size = MIN_BLK_SIZE;

    uint32_t first = ROUND_UP_DIV(
            sizeof(arena_t) + sizeof(mem_blk_t), MEM_BLK_ALIGNMENT
        ) * MEM_BLK_ALIGNMENT - sizeof(mem_blk_t);

    for (int i = 0; i < NR_MEM_BLK_DESC; i++, size *= 2) {

        uint32_t delta = ROUND_UP_DIV(
                sizeof(mem_blk_t) + size, MEM_BLK_ALIGNMENT
            ) * MEM_BLK_ALIGNMENT;
        uint32_t num = (PAGE_SIZE - first) / delta;
        uint32_t waste_per_blk =
            ((PAGE_SIZE - first) - num * delta) / num / MEM_BLK_ALIGNMENT *
            MEM_BLK_ALIGNMENT;
        uint32_t actual_size = size + waste_per_blk;
        uint32_t actual_delta = ROUND_UP_DIV(
                sizeof(mem_blk_t) + actual_size, MEM_BLK_ALIGNMENT
            ) * MEM_BLK_ALIGNMENT;

        descs[i].block_size = actual_size;
        descs[i].first = first;
        descs[i].nr_blocks_per_arena = num;
        descs[i].delta = actual_delta;

        list_init(&(descs[i].free_list));
    }
}

static mem_blk_t* arena_get_blk(arena_t *a, uint32_t idx) {
    ASSERT(a != NULL && a->desc != NULL && !((uintptr_t)a & PG_OFFSET_MASK));
    ASSERT(idx < a->desc->nr_blocks_per_arena);
    uint32_t delta = a->desc->delta;
    uintptr_t first = (uintptr_t)a + a->desc->first;
    return (mem_blk_t *)(first + idx * delta);
}

static arena_t* arena_of_blk(mem_blk_t* blk) {
    return (arena_t *)((uintptr_t)blk & PG_START_ADDRESS_MASK);
}

static void *__kmalloc(uint32_t size) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    if (size > k_mem_blk_descs[NR_MEM_BLK_DESC - 1].block_size) {
        uint32_t pg_cnt = ROUND_UP_DIV(
            size + sizeof(arena_t) + sizeof(mem_blk_t), PAGE_SIZE
        );
        arena_t *a = (arena_t *)k_get_free_pages(pg_cnt, GFP_ZERO);
        if (a == NULL) {
            return NULL;
        }
        a->cnt = pg_cnt;
        a->large = True;
        a->desc = NULL;
        mem_blk_t *mb = (mem_blk_t *)(a + 1);
        mem_blk_init(mb);
        return (void *)(mb->data_addr);
    } else {
        int idx;
        for (idx = 0; idx < NR_MEM_BLK_DESC; idx++) {
            if (k_mem_blk_descs[idx].block_size >= size) {
                break;
            }
        }
        ASSERT(idx < NR_MEM_BLK_DESC);
        mem_blk_desc_t *desc = &(k_mem_blk_descs[idx]);
        if (list_empty(&(desc->free_list))) {
            arena_t *a = (arena_t *)k_get_free_pages(1, GFP_ZERO);
            if (a == NULL) {
                return NULL;
            }
            a->desc = desc;
            a->large = False;
            a->cnt = desc->nr_blocks_per_arena;
            for (uint32_t i = 0; i < desc->nr_blocks_per_arena; i++) {
                mem_blk_t *b = arena_get_blk(a, i);
                mem_blk_init(b);
                ASSERT(!list_find(&(desc->free_list), &(b->tag)));
                list_push_back(&(desc->free_list), &(b->tag));
            }
        }
        ASSERT(!list_empty(&(desc->free_list)));
        mem_blk_t *b = __list_node_struct(
            mem_blk_t, tag, list_pop_front(&(desc->free_list))
        );
        arena_t *a = arena_of_blk(b);
        (a->cnt)--;
        return (void *)(b->data_addr);
    }
}


void *kmalloc(uint32_t size) {
    INT_STATUS old_status = disable_int();
    void *kva = __kmalloc(size);
    set_int_status(old_status);
    return kva;
}

static void __kfree(mem_blk_t *mb) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    arena_t *a = arena_of_blk(mb);
    if (a->large) {
        ASSERT(a->desc == NULL);
        k_free_pages(a, a->cnt);
    } else {
        ASSERT(a->desc != NULL);
        ASSERT(!list_find(&(a->desc->free_list), &(mb->tag)));
        list_push_front(&(a->desc->free_list), &(mb->tag));
        (a->cnt)++;
        if (a->cnt == a->desc->nr_blocks_per_arena) {
            // all blocks in a are freed, a can be freed
            for (uint32_t i = 0; i < a->desc->nr_blocks_per_arena; i++) {
                mem_blk_t *b = arena_get_blk(a, i);
                ASSERT(list_find(&(a->desc->free_list), &(b->tag)));
                list_erase( &(b->tag));
            }
            k_free_pages(a, 1);
        }
    }
}

void kfree(void *va) {
    if (va == NULL) {
        return;
    }
    mem_blk_t *mb = (mem_blk_t *)((uintptr_t)va - sizeof(mem_blk_t));
    if (mb->magic != 0x19971015 || mb->data_addr != (uintptr_t)va) {
        PANIC("bad pointer for kfree");
    }
    INT_STATUS old_status = disable_int();
    __kfree(mb);
    set_int_status(old_status);
}

void vmm_init() {
    block_desc_init(k_mem_blk_descs);
}

void vmm_print() {
    for (int i = 0; i < NR_MEM_BLK_DESC; i++) {
        kprintf(
            KPL_DEBUG,
            "blk_size=%d, list_len=%d, first=0x%x, delta=%d, num=%d\n",
            k_mem_blk_descs[i].block_size,
            list_length(&(k_mem_blk_descs[i].free_list)),
            k_mem_blk_descs[i].first, k_mem_blk_descs[i].delta,
            k_mem_blk_descs[i].nr_blocks_per_arena
        );
    }
}
