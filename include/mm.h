#ifndef __MM_H_
#define __MM_H_

#include "types.h"

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)

#define pa_to_pfn(pa) ((pa) >> PAGE_SHIFT)
#define pfn_to_pa(pfn) ((pfn) << PAGE_SHIFT)

/* clang-format off */

int			bootmem_init(void);
uint64_t	bootmem_alloc(uint64_t size);
uint64_t	bootmem_alloc_zeros(uint64_t size);
void		bootmem_free(uint64_t addr, uint64_t size);

/* clang-format on */

#endif /* __MM_H_ */
