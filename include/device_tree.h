#ifndef __DEVICE_TREE_H_
#define __DEVICE_TREE_H_

#include "types.h"

#define MAX_CPU 32

typedef struct {
    uint32_t cpu_id;
} cpu_info;

typedef struct {
    uint64_t ram_size;
} memory_info;

typedef struct {
    int cpu_num;
    cpu_info cpus[MAX_CPU];
    memory_info memory;
} device_tree;

/* clang-format off */

int parse_device_tree(uintptr_t dtb_addr);

int 		cpu_num(void);
cpu_info	cpu_of(int id);
uint64_t    ram_size(void);

/* clang-format on */

#endif /* __DEVICE_TREE_H_ */
