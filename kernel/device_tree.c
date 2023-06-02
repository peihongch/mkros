#include "device_tree.h"
#include "defs.h"
#include "libfdt/libfdt.h"

/* clang-format off */

#define ROOT_NODE		"/"
#define CPUS_NODE   	"cpus"
#define CPU_SUBNODE		"cpu@"
#define CPUMAP_SUBNODE	"cpu-map"
#define MEMORY_NODE 	"memory@"
#define SOC_NODE    	"soc"

// common props
#define PROP_ADDRESS_CELLS	"#address-cells"
#define PROP_SIZE_CELLS		"#size-cells"
#define PROP_REG    		"reg"

// cpu specific props
#define PROP_PHANDLE	"phandle"
#define PROP_STATUS		"status"
#define PROP_RISCV_ISA	"riscv,isa"
#define PROP_MMU_TYPE	"mmu-type"

device_tree dt;

int				cpu_num(void)	{ return dt.cpu_num; }
cpu_info*		cpu_of(int id)	{ return &dt.cpus[id]; }
int             mem_num(void)	{ return dt.mem_num; }
memory_info*    mem_of(int id)	{ return &dt.memory[id]; }
uint64_t		ram_size(void)	{ 
    uint64_t ram_size = 0;

    for (int i = 0; i < dt.mem_num; i++)
        ram_size += dt.memory[i].ram_size;

    return ram_size; 
}

/* clang-format on */

static uint32_t address_cells, size_cells;

// 解析 /soc 节点
static int parse_soc_node(const void* fdt, int node) {
    return 0;
}

// 解析 /cpus 节点
static int parse_cpus_node(const void* fdt, int node) {
    int cpu_offset;

    // 遍历CPU节点的子节点，获取每个CPU的信息
    fdt_for_each_subnode(cpu_offset, fdt, node) {
        const char* node_name;
        const uint32_t* reg;
        int len;

        node_name = fdt_get_name(fdt, cpu_offset, &len);
        if (node_name == NULL) {
            pr_err("failed to get subnode name of %s at %d", CPUS_NODE,
                   cpu_offset);
            continue;
        }

        if (strncmp(node_name, CPU_SUBNODE, strlen(CPU_SUBNODE)) == 0) {
            cpu_info* cpu = &dt.cpus[dt.cpu_num++];

            // 获取"reg"属性，它包含了CPU的ID
            reg = fdt_getprop(fdt, cpu_offset, PROP_REG, &len);
            if (reg == NULL) {
                pr_err("failed to get %s prop of node %s", PROP_REG, node_name);
                continue;
            }
            cpu->cpu_id = fdt32_to_cpu(*reg);

            cpu->phandle = fdt_get_phandle(fdt, cpu_offset);

            // 获取"status"属性
            const char* status =
                fdt_getprop(fdt, cpu_offset, PROP_STATUS, NULL);
            memcpy(cpu->status, status, strlen(status));

            // 获取"riscv,isa"属性
            const char* riscv_isa =
                fdt_getprop(fdt, cpu_offset, PROP_RISCV_ISA, NULL);
            memcpy(cpu->riscv_isa, riscv_isa, strlen(riscv_isa));

            // 获取"mmu-type"属性
            const char* mmu_type =
                fdt_getprop(fdt, cpu_offset, PROP_MMU_TYPE, NULL);
            memcpy(cpu->mmu_type, mmu_type, strlen(mmu_type));
        } else if (strcmp(node_name, CPUMAP_SUBNODE) == 0) {
            pr_warn("ignore subnode %s of %s", node_name, CPUS_NODE);
        } else {
            pr_warn("ignore subnode %s of %s", node_name, CPUS_NODE);
        }
    }

    return 0;
}

// 解析 /memory 节点
static int parse_memory_node(const void* fdt, int node) {
    memory_info* mem = &dt.memory[dt.mem_num++];
    const uint32_t* reg;
    int len;

    // 获取 memory 节点的 reg 属性，len 为 reg 指向区域的大小（B）
    reg = fdt_getprop(fdt, node, PROP_REG, &len);
    if (!reg) {
        pr_err("failed to get reg props of %s node: %d", MEMORY_NODE, node);
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
