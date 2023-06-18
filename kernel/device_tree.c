#include "device_tree.h"
#include "kernel.h"
#include "libfdt/libfdt.h"

/* clang-format off */

#define ROOT_NODE		    "/"
#define CPUS_NODE   	    "cpus"
#define CPU_SUBNODE		    "cpu@"
#define CPU_MAP_SUBNODE	    "cpu-map"
#define MEMORY_NODE 	    "memory@"
#define SOC_NODE    	    "soc"
#define DISTANCE_MAP_NODE   "distance-map"

// common props
#define PROP_ADDRESS_CELLS	"#address-cells"
#define PROP_SIZE_CELLS		"#size-cells"
#define PROP_REG    		"reg"
#define PROP_NUMA_NODE_ID   "numa-node-id"

// cpu specific props
#define PROP_PHANDLE	"phandle"
#define PROP_STATUS		"status"
#define PROP_RISCV_ISA	"riscv,isa"
#define PROP_MMU_TYPE	"mmu-type"
#define PROP_CPU        "cpu"

// other props
#define PROP_DISTANCE_MATRIX    "distance-matrix"

device_tree dt;

int				cpu_num(void)	            { return dt.cpu_num; }
cpu_info*		cpu_of(int id)	            { return &dt.cpus[id]; }
cpu_info*       cpu_of_phandle(int phandle) {
	for (int i = 0; i < cpu_num(); i++) {
		if (cpu_of(i)->phandle == phandle)
			return cpu_of(i);
	}
	return NULL;
}
int             mem_num(void)               { return dt.mem_num; }
memory_info*    mem_of(int id)              { return &dt.memory[id]; }
int             node_num(void)              { return dt.node_num; }
node_info*      node_of(int id)             { return &dt.nodes[id]; }
node_info*      node_of_cpu(int id)         {
    cpu_info* cpu = cpu_of(id);
    return node_of(cpu->numa_node_id);
}
int             distance_entry_num()        { return dt.distance_entry_num; }
distance*       distance_entry_of(int id)   { return &dt.distance_matrix[id]; }
uint64_t		ram_start(void)	            { return dt.memory[0].base_address; }
uint64_t		ram_size(void)	            { 
    uint64_t ram_size = 0;

    for (int i = 0; i < dt.mem_num; i++)
        ram_size += dt.memory[i].ram_size;

    return ram_size; 
}

/* clang-format on */

static uint32_t address_cells, size_cells;

static int get_prop_of_node(const void* fdt,
                            int node,
                            const char* prop,
                            uint32_t* prop_value) {
    const void* value;
    int len;

    // 获取节点属性的值
    value = fdt_getprop(fdt, node, prop, &len);
    if (!value) {
        pr_err("failed to get %s props of %s node", prop, node);
        return -1;
    }

    // 解析节点属性的值
    switch (len) {
        case 1:
            *prop_value = *(uint8_t*)value;
            break;
        case 2:
            *prop_value = be16_to_cpu(*(uint16_t*)value);
            break;
        case 4:
            *prop_value = be32_to_cpu(*(uint32_t*)value);
            break;
        default:
            pr_err("invalid len value of %s: %d", prop, len);
            break;
    }

    return 0;
}

// 解析 /soc 节点
static int parse_soc_node(const void* fdt, int node) {
    return 0;
}

// 解析 /cpus/cpu 节点
static int parse_cpu_node(const void* fdt, int node) {
    cpu_info* cpu = &dt.cpus[dt.cpu_num++];
    const uint32_t* reg;
    int len, err;

    // 获取"reg"属性，它包含了CPU的ID
    reg = fdt_getprop(fdt, node, PROP_REG, &len);
    if (reg == NULL) {
        pr_err("failed to get %s prop of node %s", PROP_REG, CPU_SUBNODE);
        return -1;
    }
    cpu->cpu_id = fdt32_to_cpu(*reg);

    cpu->phandle = fdt_get_phandle(fdt, node);

    // 获取"numa-node-id"属性
    err = get_prop_of_node(fdt, node, PROP_NUMA_NODE_ID, &cpu->numa_node_id);
    if (err < 0) {
        pr_err("failed to get %s prop of node %s", PROP_NUMA_NODE_ID,
               CPU_SUBNODE);
        return -1;
    }

    // 获取"status"属性
    const char* status = fdt_getprop(fdt, node, PROP_STATUS, NULL);
    memcpy(cpu->status, status, strlen(status));

    // 获取"riscv,isa"属性
    const char* riscv_isa = fdt_getprop(fdt, node, PROP_RISCV_ISA, NULL);
    memcpy(cpu->riscv_isa, riscv_isa, strlen(riscv_isa));

    // 获取"mmu-type"属性
    const char* mmu_type = fdt_getprop(fdt, node, PROP_MMU_TYPE, NULL);
    memcpy(cpu->mmu_type, mmu_type, strlen(mmu_type));

    return 0;
}

// 解析 /cpus/cpu-map 节点
static int parse_cpu_map_node(const void* fdt, int node) {
    int cluster_offset;

    // 遍历cpu-map的子节点，获取每个cluster的信息
    fdt_for_each_subnode(cluster_offset, fdt, node) {
        node_info* node = &dt.nodes[dt.node_num++];
        int core_offset;

        fdt_for_each_subnode(core_offset, fdt, cluster_offset) {
            const uint32_t* cpu;
            int len;

            // 获取"cpu"属性，它包含了CPU的ID
            cpu = fdt_getprop(fdt, core_offset, PROP_CPU, &len);
            if (cpu == NULL) {
                pr_err("failed to get %s prop of node %s", PROP_CPU,
                       CPU_MAP_SUBNODE);
                return -1;
            }
            node->cores[node->core_num++] = fdt32_to_cpu(*cpu);
        }
    }

    return 0;
}

// 解析 /cpus 节点
static int parse_cpus_node(const void* fdt, int node) {
    int cpu_offset;

    // 遍历CPU节点的子节点，获取每个CPU的信息
    fdt_for_each_subnode(cpu_offset, fdt, node) {
        const char* node_name;
        int len;

        node_name = fdt_get_name(fdt, cpu_offset, &len);
        if (node_name == NULL) {
            pr_err("failed to get subnode name of %s at %d", CPUS_NODE,
                   cpu_offset);
            continue;
        }

        if (strncmp(node_name, CPU_SUBNODE, strlen(CPU_SUBNODE)) == 0)
            parse_cpu_node(fdt, cpu_offset);
        else if (strcmp(node_name, CPU_MAP_SUBNODE) == 0)
            parse_cpu_map_node(fdt, cpu_offset);
        else
            pr_warn("ignore subnode %s of %s", node_name, CPUS_NODE);
    }

    return 0;
}

// 解析 /memory 节点
static int parse_memory_node(const void* fdt, int node) {
    memory_info* mem = &dt.memory[dt.mem_num++];
    const uint32_t* reg;
    int len, err;

    // 获取 memory 节点的 reg 属性，len 为 reg 指向区域的大小（B）
    reg = fdt_getprop(fdt, node, PROP_REG, &len);
    if (!reg) {
        pr_err("failed to get reg props of %s node: %d", MEMORY_NODE, node);
        return -1;
    }

    // 获取"numa-node-id"属性
    err = get_prop_of_node(fdt, node, PROP_NUMA_NODE_ID, &mem->numa_node_id);
    if (err < 0) {
        pr_err("failed to get %s prop of node %s", PROP_NUMA_NODE_ID, node);
        return -1;
    }

    // 解析 reg 属性以获取物理RAM的大小
    if (len >= sizeof(uint32_t) * (address_cells + size_cells)) {
        uint64_t base_address = 0;
        uint64_t ram_size = 0;

        // 根据address_cells和size_cells解析reg属性值
        for (int i = 0; i < address_cells; i++) {
            base_address = (base_address << 32) | fdt32_to_cpu(reg[i]);
        }

        for (int i = address_cells; i < (address_cells + size_cells); i++) {
            ram_size = (ram_size << 32) | fdt32_to_cpu(reg[i]);
        }

        // 在这里处理 Memory 相关信息，例如将其添加到内核的数据结构中
        mem->base_address = base_address;
        mem->ram_size = ram_size;
    }

    return 0;
}

// 解析 /distance-map 节点
int parse_distance_map_node(const void* fdt, int node) {
    const uint32_t* distance_matrix;
    distance* dist;
    int len;

    // 获取"distance-matrix"属性，它包含了NUMA节点之间的距离矩阵
    distance_matrix = fdt_getprop(fdt, node, PROP_DISTANCE_MATRIX, &len);
    if (distance_matrix == NULL) {
        pr_err("failed to get %s prop of node %s", PROP_DISTANCE_MATRIX,
               DISTANCE_MAP_NODE);
        return -1;
    }

    for (int i = 0; i < len / sizeof(uint32_t); i += 3) {
        dist = &dt.distance_matrix[dt.distance_entry_num++];
        dist->src = fdt32_to_cpu(distance_matrix[i]);
        dist->dst = fdt32_to_cpu(distance_matrix[i + 1]);
        dist->value = fdt32_to_cpu(distance_matrix[i + 2]);
    }

    return 0;
}

int parse_device_tree(uintptr_t dtb_addr) {
    const void* fdt = (const void*)dtb_addr;
    int root_node, offset, len;
    int err;

    // 检查设备树的有效性
    err = fdt_check_header(fdt);
    if (err != 0) {
        // 设备树无效，处理错误
        pr_err("failed to check fdt header: 0x%x, err code: %d", fdt, err);
        return err;
    }

    // 查找 / 节点
    root_node = fdt_path_offset(fdt, ROOT_NODE);
    if (root_node < 0) {
        pr_err("failed to check get %s path offset, err code: %d", ROOT_NODE,
               root_node);
        return -1;
    }

    // 清空设备树信息
    memset(&dt, 0, sizeof(device_tree));

    // 获取 #address-cells 属性的值
    address_cells = fdt_address_cells(fdt, root_node);
    if (address_cells < 0) {
        pr_err("failed to get %s of %s node", PROP_ADDRESS_CELLS, root_node);
        return -1;
    }

    // 获取 #size-cells 属性的值
    size_cells = fdt_size_cells(fdt, root_node);
    if (size_cells < 0) {
        pr_err("failed to get %s of %s node", PROP_SIZE_CELLS, root_node);
        return -1;
    }

    // 遍历根节点的子节点，查找感兴趣的节点
    fdt_for_each_subnode(offset, fdt, root_node) {
        const char* node_name;

        node_name = fdt_get_name(fdt, offset, &len);
        if (node_name == NULL) {
            pr_err("failed to get node name of %d", offset);
            continue;
        }

        if (strncmp(node_name, MEMORY_NODE, strlen(MEMORY_NODE)) == 0) {
            err = parse_memory_node(fdt, offset);
        } else if (strcmp(node_name, CPUS_NODE) == 0) {
            err = parse_cpus_node(fdt, offset);
        } else if (strcmp(node_name, DISTANCE_MAP_NODE) == 0) {
            err = parse_distance_map_node(fdt, offset);
        } else if (strcmp(node_name, SOC_NODE) == 0) {
            err = parse_soc_node(fdt, offset);
        } else {
            pr_warn("ignore node %s", node_name);
            err = 0;
        }

        if (err) {
            pr_err("failed to parse node %s, err code: %d", node_name, err);
        }
    }

    return 0;
}
