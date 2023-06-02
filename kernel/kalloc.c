// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "defs.h"
#include "device_tree.h"
#include "memlayout.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "types.h"

void freerange(void* pa_start, void* pa_end);

// defined by kernel.ld.
extern char kernel_start[];  // kernel start address
extern char kernel_end[];    // first address after kernel.

struct run {
    struct run* next;
};

struct {
    struct spinlock lock;
    struct run* freelist;
    uint64_t npage;
    uint64_t ram_start;
    uint64_t ram_avail_start;
    uint64_t ram_size;
    uint64_t ram_avail_size;
    uint64_t ram_stop;
    uint64_t kernel_start;
    uint64_t kernel_end;
} kmem;

void kinit() {
    initlock(&kmem.lock, "kmem");
    kmem.freelist = 0;
    kmem.npage = 0;
    kmem.kernel_start = (uint64_t)kernel_start;
    kmem.kernel_end = (uint64_t)kernel_end;
    kmem.ram_start = ram_start();
    kmem.ram_avail_start = (uint64_t)kernel_end;
    kmem.ram_size = ram_size();
    kmem.ram_stop = kmem.ram_start + kmem.ram_size;
    kmem.ram_avail_size = kmem.ram_stop - kmem.ram_avail_start;
    freerange((void*)kmem.ram_avail_start, (void*)kmem.ram_stop);
    pr_info("kernel_end: %p, phystop: %p", kernel_end, (void*)kmem.ram_stop);
    pr_info("kinit");
}

void freerange(void* pa_start, void* pa_end) {
    char* p;
    p = (char*)PGROUNDUP((uint64_t)pa_start);
    for (; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
        kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void* pa) {
    struct run* r;

    if (((uint64_t)pa % PGSIZE) != 0 || (uint64_t)pa < kmem.kernel_end ||
        (uint64_t)pa >= kmem.ram_stop)
        panic("kfree");

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    kmem.npage++;
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void* kalloc(void) {
    struct run* r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if (r) {
        kmem.freelist = r->next;
        kmem.npage--;
    }
    release(&kmem.lock);

    if (r)
        memset((char*)r, 5, PGSIZE);  // fill with junk
    return (void*)r;
}

uint64_t freemem_amount(void) {
    return kmem.npage << PGSHIFT;
}
