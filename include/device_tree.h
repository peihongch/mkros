#ifndef __DEVICE_TREE_H_
#define __DEVICE_TREE_H_

#include "types.h"

#define MAX_CPU 32             // maximum number of CPUs
#define MAX_MEM 8              // maximum number of memory regions
#define MAX_NUMA_NODE MAX_MEM  // maximum number of numa nodes
#define MAX_CORES_PER_NODE 4   // maximum number of cores per numa node
#define MAX_INFO_STR_LEN 16
#define MAX_DISTANCE_MAP_ENTRY 64  // maxmum number of distance map entry (8*8)

typedef struct {
    int core_num;
    uint32_t cores[MAX_CORES_PER_NODE];  // cpu phandle list of the node
} node_info;

typedef struct {
    uint32_t phandle;
    uint32_t cpu_id;
    uint32_t numa_node_id;
    char status[MAX_INFO_STR_LEN];
    char riscv_isa[MAX_INFO_STR_LEN];
    char mmu_type[MAX_INFO_STR_LEN];
} cpu_info;

typedef struct {
    uint32_t numa_node_id;
    uint64_t base_address;
    uint64_t ram_size;
} memory_info;

typedef struct {
    uint32_t src;
    uint32_t dst;
    uint32_t value;
} distance;

typedef struct {
    int cpu_num;
    cpu_info cpus[MAX_CPU];
    int mem_num;
    memory_info memory[MAX_MEM];
    int node_num;
    node_info nodes[MAX_CORES_PER_NODE];
    int distance_entry_num;
    distance distance_matrix[MAX_DISTANCE_MAP_ENTRY];
} device_tree;

/* clang-format off */

int parse_device_tree(uintptr_t dtb_addr);

int 		    cpu_num(void);
cpu_info*	    cpu_of(int id);
cpu_info*       cpu_of_phandle(int phandle);
uint64_t		ram_start(void);
uint64_t        ram_size(void);
uint64_t        ram_end(void);
int             mem_num(void);
memory_info*    mem_of(int id);
int             distance_entry_num();
distance*       distance_entry_of(int id);
int             node_num(void);
node_info*      node_of(int id);
node_info*      node_of_cpu(int id);

#define for_each_cpu(id, cpu) \
    for (id = 0; cpu = cpu_of(id), id < cpu_num(); id++)

#define for_each_mem(id, mem) \
    for (id = 0; mem = mem_of(id), id < mem_num(); id++)

#define for_each_node(id, node) \
    for (id = 0; node = node_of(id), id < node_num(); id++)

#define for_each_cpu_of_node(id, node, cpu) \
    for (id = 0;                            \
         cpu = cpu_of_phandle((node)->cores[id]), id < (node)->core_num; id++)

#define for_each_distance_entry(id, de) \
    for (id = 0; de = distance_entry_of(id), id < distance_entry_num(); id++)

/* clang-format on */

#endif /* __DEVICE_TREE_H_ */
