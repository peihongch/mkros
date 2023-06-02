#ifndef __DEVICE_TREE_H_
#define __DEVICE_TREE_H_

#include "types.h"

#define MAX_CPU 32  // maximum number of CPUs
#define MAX_MEM 8   // maximum number of memory regions
#define MAX_INFO_STR_LEN 16

typedef struct {
    uint32_t cpu_id;
    uint32_t phandle;
    char status[MAX_INFO_STR_LEN];
    char riscv_isa[MAX_INFO_STR_LEN];
    char mmu_type[MAX_INFO_STR_LEN];
} cpu_info;

typedef struct {
    uint64_t base_address;
    uint64_t ram_size;
} memory_info;

typedef struct {
    int cpu_num;
    cpu_info cpus[MAX_CPU];
    int mem_num;
    memory_info memory[MAX_MEM];
} device_tree;

/* clang-format off */

int parse_device_tree(uintptr_t dtb_addr);

int 		    cpu_num(void);
cpu_info*	    cpu_of(int id);
uint64_t		ram_start(void);
uint64_t        ram_size(void);
int             mem_num(void);
memory_info*    mem_of(int id);

#define for_each_cpu(id, cpu) \
    for (id = 0, cpu = cpu_of(id); id < cpu_num(); id++)

#define for_each_mem(id, mem) \
    for (id = 0, mem = mem_of(id); id < mem_num(); id++)

/* clang-format on */

#endif /* __DEVICE_TREE_H_ */
