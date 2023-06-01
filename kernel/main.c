#include "defs.h"
#include "libfdt/libfdt.h"
#include "memlayout.h"
#include "param.h"
#include "riscv.h"
#include "sbi.h"
#include "types.h"

static inline void inithartid(unsigned long hartid) {
    w_tp(hartid);
}

volatile static int started = 0;

extern char stext[];
extern char etext[];
extern char srodata[];
extern char erodata[];
extern char sdata[];
extern char edata[];
extern char sbss[];
extern char ebss[];
extern char end[];

extern void _entry(void);

void* fdt;  // 设备树的起始地址

void sys_info();
unsigned long get_ram_size(void);

// start() jumps here in supervisor mode on all CPUs.
int main(unsigned long hartid, unsigned long dtb_pa) {
    inithartid(hartid);
    if (hartid == 0) {
        fdt = (void*)dtb_pa;
        consoleinit();
        printkinit();
        sys_info();
        pr_info("");
        pr_info("hart %d enter main()...", hartid);
        kinit();         // physical page allocator
        timerinit();     // init a lock for timer
        trapinithart();  // install kernel trap vector, including interrupt
                         // handler
        pr_info("hart %d init done", hartid);
        pr_info("");

        for (int i = 1; i < NCPU; i++) {
            unsigned long mask = 1 << i;
            sbi_ecall_hart_start(i, _entry, 0);
            sbi_ecall_send_ipi(mask, 0);
        }
        __sync_synchronize();
        started = 1;
        pr_info("mkros kernel started");
    } else {
        sbi_ecall_hart_start(0, _entry, 0);
        sbi_ecall_send_ipi(0, 0);
        while (started == 0)
            ;
        __sync_synchronize();
        pr_info("hart %d enter main()...", hartid);
        trapinithart();
        pr_info("hart %d init done", hartid);
    }

    while (1) {
    }

    return 0;
}

void sys_info() {
    unsigned long ram_size = get_ram_size();

    /* clang-format off */
    printk("\n");
    printk("  __  __   _  __  ____     ___    ____     __              ____  ___ ____   ____   __     __\n");
    printk(" |  \\/  | | |/ / |  _ \\   / _ \\  / ___|   / _| ___  _ __  |  _ \\|_ _/ ___| / ___|  \\ \\   / /\n");
    printk(" | |\\/| | | ' /  | |_) | | | | | \\___ \\  | |_ / _ \\| '__| | |_) || |\\___ \\| |   ____\\ \\ / / \n");
    printk(" | |  | | | . \\  |  _ <  | |_| |  ___) | |  _| (_) | |    |  _ < | | ___) | |__|_____\\ V /  \n");
    printk(" |_|  |_| |_|\\_\\ |_| \\_\\  \\___/  |____/  |_|  \\___/|_|    |_| \\_\\___|____/ \\____|     \\_/   \n");
    printk("\n");
    /* clang-format on */

    printk("text section:\t[%p ~ %p]\n", stext, etext);
    printk("rodata section:\t[%p ~ %p]\n", srodata, erodata);
    printk("data section:\t[%p ~ %p]\n", sdata, edata);
    printk("bss section: \t[%p ~ %p]\n", sbss, ebss);
    printk("PhyMem Avail:\t[%p ~ %p]\n", end, (void*)PHYSTOP);
    printk("\n");
    printk("RAM Size:\t%dMB\n", ram_size >> 20);
    printk("\n");
}

/*

memory@80000000 {
    device_type = "memory";
    reg = <0x00 0x80000000 0x00 0x8000000>;
};

在设备树（Device Tree）中，reg属性用于定义设备的地址范围。上述示例中，reg属性包含四个字段，每个字段的含义如下：

- 第一个字段（0x00）：指定地址的起始偏移量（offset）。它表示设备在父设备地址空间中的偏移量，也就是相对于父设备的基地址的偏移量。
- 第二个字段（0x80000000）：指定设备的基地址（base address）。它是设备的起始物理地址，表示设备在整个地址空间中的位置。
- 第三个字段（0x00）：指定地址的起始偏移量（offset）。它类似于第一个字段，用于表示设备在父设备地址空间中的偏移量。
- 第四个字段（0x8000000）：指定设备的大小（size）。它表示设备在地址空间中占用的字节数。

因此，根据提供的reg属性，设备的地址范围从基地址0x80000000开始，大小为0x8000000字节，相对于父设备的起始偏移量为0x00。

*/
unsigned long get_ram_size(void) {
    int memory_node;
    const uint32_t* reg;
    int len, err;
    unsigned long ram_size = 0;

    // 检查设备树的有效性
    err = fdt_check_header(fdt);
    if (err != 0) {
        pr_error("failed to check fdt header: 0x%x, err code: %d", fdt, err);
        return 0;
    }

    // 查找 memory 节点
    memory_node = fdt_path_offset(fdt, "/memory");
    if (memory_node < 0) {
        pr_error("failed to check get /memory path offset, err code: %d",
                 memory_node);
        return 0;
    }

    // 获取 memory 节点的 reg 属性，len 为 reg 指向区域的大小（B）
    reg = fdt_getprop(fdt, memory_node, "reg", &len);
    if (!reg) {
        pr_error("failed to get reg props of /memory node: %d", memory_node);
        return 0;
    }

    // 解析 reg 属性以获取物理RAM的大小
    ram_size = fdt32_to_cpu(reg[3]);

    return ram_size;
}
