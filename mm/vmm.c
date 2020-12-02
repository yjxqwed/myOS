#include <mm/vmm.h>
#include <common/debug.h>
#include <arch/x86.h>
// #include <mm/pager.h>
// #include <mm/pmem.h>
// // #include <bitmap.h>
#include <kprintf.h>
// // #include <common/types.h>
// #include <common/debug.h>
// #include <string.h>

// extern void *kernel_image_end;
// // _end is the (physical) end of the kernel binary image
// static uintptr_t _end = __pa(&kernel_image_end);

// #define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))
// void print_mem_info(multiboot_info_t *mbi) {
//     kprintf(KPL_NOTICE, "================ Memory Info ================\n");
//     kprintf(KPL_NOTICE, "Flags = 0x%x\n", (uint32_t)mbi->flags);

//     // check mem
//     if (CHECK_FLAG(mbi->flags, 0)) {
//         kprintf(
//             KPL_NOTICE, "mem_lower = 0x%xKB, mem_upper = 0x%xKB\n",
//             (uint32_t)mbi->mem_lower, (uint32_t)mbi->mem_upper
//         );
//     }

//     // check mmap
//     if (CHECK_FLAG (mbi->flags, 6)) {
//         kprintf(
//             KPL_NOTICE, "mmap_addr = 0x%X, mmap_length = 0x%X\n",
//             (uint32_t)mbi->mmap_addr, (uint32_t)mbi->mmap_length
//         );
//         for (
//             multiboot_memory_map_t *mmap =
//                 (multiboot_memory_map_t *)mbi->mmap_addr;

//             (uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length;

//             mmap = (multiboot_memory_map_t *)(
//                 (uint32_t)mmap + mmap->size + sizeof(mmap->size)
//             )
//         ) {
//             kprintf(
//                 KPL_NOTICE, 
//                 " size = 0x%x, base_addr = 0x%X,"
//                 " length = 0x%X, type = 0x%x\n",
//                 (uint32_t) mmap->size,
//                 // (uint32_t) (mmap->addr >> 32),
//                 (uint32_t) (mmap->addr & 0xffffffff),
//                 // (uint32_t) (mmap->len >> 32),
//                 (uint32_t) (mmap->len & 0xffffffff),
//                 (uint32_t) mmap->type
//             );
//         }
//     }
//     kprintf(KPL_DEBUG, "_end=0x%X\n", _end);
//     kprintf(KPL_NOTICE, "=============================================\n");
// }

// // not always return
// // if pass memory check, return the max page frame number
// static uint32_t check_memory(multiboot_info_t *mbi) {
//     if (CHECK_FLAG(mbi->flags, 0)) {
//         // mbi->mem_upper is given in KiB
//         uint32_t high_free_mem = mbi->mem_upper * 1024;
//         if (high_free_mem < MEM_LIMIT) {
//             kprintf(
//                 KPL_PANIC, 
//                 "Not Enough Memory; "
//                 "try with >= 256 MiB mem. System Halted.\n"
//             );
//             while (1);
//         }
//         uint32_t max_pfn = __page_number(HIGH_MEM_BASE + high_free_mem - 1);
//         return max_pfn;
//     } else {
//         kprintf(KPL_PANIC, "No Memory Info. System Halted.\n");
//         while (1);
//     }
//     // never reach here
//     return 0;
// }

// // kernel physical memory page pool
// static mpp_t kppool;
// // user physical memory page pool
// static mpp_t uppool;

// // virtual memory area of kernel's heap.
// static vma_t kernel_heap;

// static void init_pool(
//     mpp_t *pool, uint32_t start_page_no,
//     uint32_t num_pages, uint32_t btmp_base
// ) {
//     pool->num_total_pages = num_pages;
//     pool->start_page_number = start_page_no;
//     pool->btmp.bits_ = (uint8_t *)btmp_base;
//     // TODO: what if num_pages % 8 != 0?
//     bitmap_init(&(pool->btmp), num_pages / 8);
// }

// static void print_pool(const mpp_t *pool, const char* name) {
//     kprintf(
//         KPL_DUMP,
//         "%s: { start_page_number = 0x%x, num_total_pages = 0x%x, "
//         "btmp = { byte_num = 0x%x, bits = 0x%X, "
//         "first_zero_bit = 0x%x, num_zero = 0x%x} }\n",
//         name, pool->start_page_number, pool->num_total_pages,
//         pool->btmp.byte_num_, (uint32_t)pool->btmp.bits_,
//         (uint32_t)pool->btmp.first_zero_bit, pool->btmp.num_zero
//     );
// }

// // init the pmem pools
// // @param num_pages is the total page from 0x00100000
// // static void init_ppools(uint32_t num_free_pages) {
// //     uint32_t num_pages = num_free_pages + AVAIL_PAGE_BASE;
// //     uint32_t half_num_pages = num_pages / 2;
// //     uint32_t k_free_pages = 0;
// //     uint32_t k_needed_pages = __page_number(K_NEEDED_MEM);
// //     uint32_t free_page_no = __page_number(FREE_MEM_BASE);
// //     if (half_num_pages > k_needed_pages) {
// //         k_free_pages = k_needed_pages - free_page_no;
// //     } else {
// //         k_free_pages = half_num_pages - free_page_no;
// //     }
// //     uint32_t u_free_pages = num_pages - free_page_no - k_free_pages;
// //     kprintf(KPL_NOTICE, "\nkfp: %x; ufp: %x\n", k_free_pages, u_free_pages);
// //     init_pool(&kppool, free_page_no, k_free_pages, KPPOOL_BTMP_BASE_ADDR);
// //     // print_pool(&kppool, "kernel ppool");
// //     init_pool(
// //         &uppool, free_page_no + k_free_pages,
// //         u_free_pages, UPPOOL_BTMP_BASE_ADDR
// //     );
// //     // print_pool(&uppool, "user ppool");
// // }

// // static void init_vpools() {
// //     // init kvpool
// //     init_pool(
// //         &kvpool, __page_number(KERNEL_HEAP_BASE_ADDR),
// //         kppool.num_total_pages, KVPOOL_BTMP_BASE_ADDR
// //     );
// //     // print_pool(&kvpool, "kernel vpool");
// // }

// // void init_kernel_heap() {
// //     kernel_heap.vm_start = KERNEL_HEAP_BASE_ADDR;
// //     kernel_heap.vm_size_in_page = kppool.num_total_pages;
// //     kernel_heap.vm_end =
// //         kernel_heap.vm_start + kernel_heap.vm_size_in_page * PAGE_SIZE - 1;
// //     kernel_heap.btmp.bits_ = (uint8_t *)KVPOOL_BTMP_BASE_ADDR;
// //     bitmap_init(&(kernel_heap.btmp), kernel_heap.vm_size_in_page / 8);
// // }


// void mm_init() {
    
// }

// // void *vm_kernel_get_pages(uint32_t page_cnt) {
// //     // allocate at most 256 pages in one request
// //     if (page_cnt == 0 || page_cnt > 256) {
// //         return NULL;
// //     }
    
// // }

// // void *get_ppage(mpp_t *pool) {
// //     btmp_t *btmp = &(pool->btmp);
// //     int bit_idx = bitmap_scan(btmp, 1);
// //     if (bit_idx == -1) {
// //         return NULL;
// //     }
// //     bitmap_set(btmp, bit_idx, 1);
// //     ASSERT(bitmap_bit_test(btmp, bit_idx));
// //     return (void *)(
// //         (pool->start_page_number + (uint32_t)bit_idx) * PAGE_SIZE
// //     );
// // }

// // static int pool_has_enough_free_pages(mpp_t *pool, uint32_t pg_cnt) {
// //     return pool->btmp.num_zero >= pg_cnt;
// // }

// // map vpfn -> ppfn
// // static void bind_page(int vpfn, int ppfn) {

// // }

// // void *kernel_vmalloc(uint32_t pg_cnt) {
// //     if (pg_cnt > 256) {
// //         return NULL;
// //     }
// //     // check that the physical page pool has enough pages
// //     if (!pool_has_enough_free_pages(&kppool, pg_cnt)) {
// //         return NULL;
// //     }
// //     void *vaddr_start = get_vaddr(&kernel_heap, pg_cnt);
// //     if (vaddr_start == NULL) {
// //         return NULL;
// //     }
// //     uint32_t vpfn = __page_number(vaddr_start);
// //     for (int i = 0; i < pg_cnt; i++) {
// //         void *p = get_ppage(&kppool);
// //         ASSERT(p != NULL);
// //         uint32_t ppfn = __page_number(p);
// //         bind_page(vpfn + i, ppfn);
// //     }
// // }

// // void *get_vaddr(vma_t *vma, uint32_t pg_cnt) {
// //     int bit_idx = bitmap_scan(&(vma->btmp), pg_cnt);
// //     if (bit_idx == -1) {
// //         return NULL;
// //     }
// //     for (int i = 0; i < pg_cnt; i++) {
// //         bitmap_set(&(vma->btmp), bit_idx + i, 1);
// //         ASSERT(bitmap_bit_test(&(vma->btmp), bit_idx + i));
// //     }
// //     return (void *)(vma->vm_start + (uint32_t)bit_idx * PAGE_SIZE);
// // }


void *k_get_free_page(uint32_t gfp_flags) {
    ppage_t *fp = page_alloc(gfp_flags);
    if (!fp) {
        return NULL;
    }
    page_incref(fp);
    return page2kva(fp);
}


void k_free_page(void *kva) {
    // kva has to be page alligned
    ASSERT(!((uintptr_t)kva & PG_OFFSET_MASK));
    ppage_t *p = kva2page(kva);
    // kprintf(KPL_DEBUG, "kva=0x%X, p->num_ref=%d\n", kva, p->num_ref);
    ASSERT(p->num_ref == 1);
    page_decref(p);
}


void *k_get_free_pages(uint32_t pgcnt, uint32_t gfp_flags) {
    return NULL;
}


void k_free_pages(void *kva, uint32_t pgcnt) {
    return;
}