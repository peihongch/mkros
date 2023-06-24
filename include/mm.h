#ifndef __MM_H_
#define __MM_H_

#include "riscv.h"
#include "types.h"

#define pa_to_pfn(pa) ((pa) >> PGSHIFT)
#define pfn_to_pa(pfn) ((pfn) << PGSHIFT)

/* clang-format off */

// physical pages allocator APIs
extern void* (* alloc_pages)(uint32_t npages);
extern void* (* alloc_zero_pages)(uint32_t npages);
extern void  (* free_pages)(void* addr, uint32_t npages);

#define alloc_pages_exact(size) alloc_pages(((size) + PGSIZE - 1) >> PGSHIFT)
#define zalloc_pages_exact(size) \
    alloc_zero_pages(((size) + PGSIZE - 1) >> PGSHIFT)
#define free_pages_exact(addr, size) \
    free_pages((addr), ((size) + PGSIZE - 1) >> PGSHIFT)

// bootmem APIs
int		bootmem_init(void);
void*	bootmem_alloc(uint32_t npages);
void*	bootmem_alloc_zeros(uint32_t npages);
void	bootmem_free(void* addr, uint32_t npages);

/* clang-format on */

#endif /* __MM_H_ */
