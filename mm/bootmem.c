// Physical memory allocator, for early kernel boot stage.
// Allocates one or more consecutive physical 4096-byte pages at a time.

#include "device_tree.h"
#include "kernel.h"
#include "list.h"
#include "math.h"
#include "mm.h"
#include "spinlock.h"

void* (* alloc_pages)(uint32_t npages);
void* (* alloc_zero_pages)(uint32_t npages);
void  (* free_pages)(void* addr, uint32_t npages);

typedef struct {
    struct list_head list;
    struct spinlock lock;
    uint64_t start_pfn;
    uint64_t npages;
    uint32_t next_offset;
    uint64_t* bitmap;
} bootmem_node;

static bootmem_node* bootmem_all_nodes[MAX_NUMA_NODE];

static inline void register_mm_handlers(void) {
    alloc_pages = bootmem_alloc;
    alloc_zero_pages = bootmem_alloc_zeros;
    free_pages = bootmem_free;
}

int bootmem_init(void) {
    bootmem_node* node;
    memory_info* mem;
    uint64_t start_addr, end_addr;
    uint32_t bitmap_size, bitmap_npages;
    int id;

    for_each_mem(id, mem) {
        start_addr = max(mem->base_address, (uint64_t)kernel_end);
        end_addr = start_addr + mem->ram_size;

        // alloc the last page of mem in this numa node for `bootmem_node`,
        // which will be reclaimed after destoying the bootmem.
        node = (bootmem_node*)(end_addr - PGSIZE);
        bootmem_all_nodes[id] = node;

        INIT_LIST_HEAD(&node->list);
        initlock(&node->lock, "bootmem");
        node->npages = mem->ram_size >> PGSHIFT;
        node->start_pfn = pa_to_pfn(start_addr);

        bitmap_size = (node->npages + 63) / 64 * sizeof(*(node->bitmap));
        bitmap_npages = (bitmap_size + PGSIZE - 1) >> PGSHIFT;
        if (bitmap_npages + 1 > node->npages) {
            pr_err("not enough memory space for bootmem bitmap.");
            return -1;
        }
        node->bitmap =
            (uint64_t*)(((uint64_t)node) - (bitmap_npages << PGSHIFT));
        memset(node->bitmap, 0, bitmap_size);
        node->next_offset = 0;

        // mark pages of bootmem_node with related bitmap as used.
        // use `for loop` to make this simple.
        uint32_t off = pa_to_pfn((uint64_t)node->bitmap) - node->start_pfn;
        uint32_t rest = 64 - (off & 63);
        node->bitmap[off >> 6] |= ((1 << rest) - 1);
        off += rest;
        if (off < node->npages)
            memset(&node->bitmap[off >> 6], 1, (node->npages - off) / 8);
    }

    register_mm_handlers();

    return 0;
}

void* bootmem_alloc(uint32_t npages) {
    cpu_info* cpu = cpu_of(cpu_id());
    bootmem_node* node = bootmem_all_nodes[cpu->numa_node_id];
    uint64_t phy_addr = 0;
    int retry = 1;

    acquire(&node->lock);

    if (npages >= node->npages)
        goto out;

    int start, end;
repeat:
    if (node->next_offset + npages < node->npages) {
        start = node->next_offset;
        end = start + npages;
        while (start < end) {
            int row = start;
            int col = do_div(row, 64);
            if (node->bitmap[row] & (1 << (63 - col))) {
                // current page already allocated
                node->next_offset = start + 1;
                goto repeat;
            }
            start++;
        }

        // now we found the usable continuous pages,
        // calculate the start address to return and fill the bitmap
        phy_addr = (node->start_pfn + node->next_offset) << PGSHIFT;
        while (node->next_offset < end) {
            int row = node->next_offset;
            int col = do_div(row, 64);
            node->bitmap[row] |= (1UL << (63 - col));
            node->next_offset++;
        }
    } else if (retry) {
        node->next_offset = 0;
        retry = 0;
        goto repeat;
    } else {
        pr_warn("no enough space of numa node %d to allocate %d pages.",
                cpu->numa_node_id, npages);
    }

out:
    release(&node->lock);

    return (void*)phy_addr;
}

void* bootmem_alloc_zeros(uint32_t npages) {
    void* phy_addr = bootmem_alloc(npages);
    memset(phy_addr, 0, npages << PGSHIFT);
    return phy_addr;
}

void bootmem_free(void* addr, uint32_t npages) {
    if ((uint64_t)addr & (PGSIZE - 1))
        panic("addr of bootmem_free should be aligned to PGSIZE(4k).");

    cpu_info* cpu = cpu_of(cpu_id());
    bootmem_node* node = bootmem_all_nodes[cpu->numa_node_id];
    uint64_t pfn = ((uint64_t)addr) >> PGSHIFT;

    acquire(&node->lock);

    for (int i = 0; i < npages; i++) {
        int row = (pfn++) - node->start_pfn;
        int col = do_div(row, 64);
        node->bitmap[row] &= ~(1 << (63 - col));
    }

    release(&node->lock);
}
