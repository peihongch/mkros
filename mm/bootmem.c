// Physical memory allocator, for early kernel boot stage.
// Allocates one or more consecutive physical 4096-byte pages at a time.

#include "defs.h"
#include "device_tree.h"
#include "list.h"
#include "mm.h"
#include "spinlock.h"

typedef struct {
    struct list_head list;
    struct spinlock lock;
    uint64_t start_pfn;
    uint64_t npages;
    uint32_t next_offset;
    uint64_t* bitmap;
} bootmem_node;

static bootmem_node* bootmem_all_nodes[MAX_NUMA_NODE];

int bootmem_init(void) {
    bootmem_node* node;
    memory_info* mem;
    uint64_t start_addr, end_addr;
    uint32_t bitmap_size, bitmap_npages;
    int id;

    for_each_mem(id, mem) {
        start_addr = mem->base_address;
        end_addr = start_addr + mem->ram_size;

        // alloc the last page of mem in this numa node for `bootmem_node`,
        // which will be reclaimed after destoying the bootmem.
        node = (bootmem_node*)(end_addr - PAGE_SIZE);
        bootmem_all_nodes[id] = node;

        INIT_LIST_HEAD(&node->list);
        initlock(&node->lock, "bootmem");
        node->npages = mem->ram_size >> PAGE_SHIFT;
        node->start_pfn = pa_to_pfn(mem->base_address);

        bitmap_size = (node->npages + 63) / 64 * sizeof(*(node->bitmap));
        bitmap_npages = (bitmap_size + PAGE_SIZE - 1) >> PAGE_SHIFT;
        if (bitmap_npages + 1 > node->npages) {
            pr_err("not enough memory space for bootmem bitmap.");
            return -1;
        }
        node->bitmap =
            (uint64_t*)(((uint64_t)node) - (bitmap_npages << PAGE_SHIFT));
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

    return 0;
}

uint64_t bootmem_alloc(uint64_t size) {
    return 0;
}

uint64_t bootmem_alloc_zeros(uint64_t size) {
    return 0;
}

void bootmem_free(uint64_t addr, uint64_t size) {}
