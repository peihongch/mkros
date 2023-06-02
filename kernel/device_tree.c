#include "device_tree.h"
#include "defs.h"
#include "libfdt/libfdt.h"

/* clang-format off */

#define ROOT_NODE	"/"
#define CPUS_NODE   "/cpus"
#define MEMORY_NODE "/memory"

#define PROP_REG    		"reg"
#define PROP_ADDRESS_CELLS	"#address-cells"
#define PROP_SIZE_CELLS		"#size-cells"

device_tree dt;

int			cpu_num(void)	{ return dt.cpu_num; }
cpu_info	cpu_of(int id)	{ return dt.cpus[id]; }
uint64_t	ram_size(void)	{ return dt.memory.ram_size; }

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

// 解析 /cpus 节点
static int parse_cpus_node(const void* fdt) {
    int off, cpus_offset;

    // 寻找CPU节点
    cpus_offset = fdt_path_offset(fdt, CPUS_NODE);
    if (cpus_offset < 0) {
        // 未找到CPU节点，处理错误
        pr_err("%s node not found, err code: %d", CPUS_NODE, cpus_offset);
        return -1;
    }

    dt.cpu_num = 0;

    // 遍历CPU节点的子节点，获取每个CPU的信息
    fdt_for_each_subnode(off, fdt, cpus_offset) {
        cpu_info* cpu = &dt.cpus[off];
        const uint32_t* reg;
        int len;

        // 获取"reg"属性，它包含了CPU的ID
        reg = fdt_getprop(fdt, off, PROP_REG, &len);
        if (reg == NULL) {
            // 未找到"reg"属性，处理错误
            continue;
        }

        // 获取CPU ID
        uint32_t cpu_id = fdt32_to_cpu(*reg);

        // 在这里处理CPU相关信息，例如将其添加到内核的数据结构中
        dt.cpu_num++;
        cpu->cpu_id = cpu_id;
    }

    return 0;
}

// 解析 /memory 节点
static int parse_memory_node(const void* fdt) {
    memory_info* mem = &dt.memory;
    uint64_t ram_size;
    int memory_node;
    const uint32_t* reg;
    int len, err;

    // 检查设备树的有效性
    err = fdt_check_header(fdt);
    if (err != 0) {
        pr_err("failed to check fdt header: 0x%x, err code: %d", fdt, err);
        return -1;
    }

    // 查找 memory 节点
    memory_node = fdt_path_offset(fdt, MEMORY_NODE);
    if (memory_node < 0) {
        pr_err("failed to check get %s path offset, err code: %d", MEMORY_NODE,
               memory_node);
        return -1;
    }

    // 获取 memory 节点的 reg 属性，len 为 reg 指向区域的大小（B）
    reg = fdt_getprop(fdt, memory_node, PROP_REG, &len);
    if (!reg) {
        pr_err("failed to get reg props of %s node: %d", MEMORY_NODE,
               memory_node);
        return -1;
    }

    // 解析 reg 属性以获取物理RAM的大小
    ram_size = fdt32_to_cpu(reg[3]);

    // 在这里处理 Memory 相关信息，例如将其添加到内核的数据结构中
    mem->ram_size = ram_size;

    return 0;
}

static int parse_root_node(const void* fdt) {
    int root_node;

    // 查找 / 节点
    root_node = fdt_path_offset(fdt, ROOT_NODE);
    if (root_node < 0) {
        pr_err("failed to check get %s path offset, err code: %d", ROOT_NODE,
               root_node);
        return -1;
    }

    // 获取 #address-cells 属性的值
    get_prop_of_node(fdt, root_node, PROP_ADDRESS_CELLS, &address_cells);

    // 获取 #size-cells 属性的值
    get_prop_of_node(fdt, root_node, PROP_SIZE_CELLS, &size_cells);

    return 0;
}

int parse_device_tree(uintptr_t dtb_addr) {
    const void* fdt = (const void*)dtb_addr;
    int err;

    // 检查设备树的有效性
    err = fdt_check_header(fdt);
    if (err != 0) {
        // 设备树无效，处理错误
        pr_err("failed to check fdt header: 0x%x, err code: %d", fdt, err);
        return err;
    }

    parse_root_node(fdt);

    parse_cpus_node(fdt);

    parse_memory_node(fdt);

    return 0;
}
