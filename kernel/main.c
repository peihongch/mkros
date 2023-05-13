#include "defs.h"
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

void sys_info();

// start() jumps here in supervisor mode on all CPUs.
int main(unsigned long hartid, unsigned long dtb_pa) {
    inithartid(hartid);
    if (hartid == 0) {
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
    /* clang-format off */
    
    printk("\n");
    printk("  __  __   _  __  ____     ___    ____     __              ____  ___ ____   ____   __     __\n");
    printk(" |  \\/  | | |/ / |  _ \\   / _ \\  / ___|   / _| ___  _ __  |  _ \\|_ _/ ___| / ___|  \\ \\   / /\n");
    printk(" | |\\/| | | ' /  | |_) | | | | | \\___ \\  | |_ / _ \\| '__| | |_) || |\\___ \\| |   ____\\ \\ / / \n");
    printk(" | |  | | | . \\  |  _ <  | |_| |  ___) | |  _| (_) | |    |  _ < | | ___) | |__|_____\\ V /  \n");
    printk(" |_|  |_| |_|\\_\\ |_| \\_\\  \\___/  |____/  |_|  \\___/|_|    |_| \\_\\___|____/ \\____|     \\_/   \n");
    printk("\n");
    printk("text section:\t[%p ~ %p]\n", stext, etext);
    printk("rodata section:\t[%p ~ %p]\n", srodata, erodata);
    printk("data section:\t[%p ~ %p]\n", sdata, edata);
    printk("bss section: \t[%p ~ %p]\n", sbss, ebss);
    printk("PhyMem Avail:\t[%p ~ %p]\n", end, (void*)PHYSTOP);
    printk("\n");

    /* clang-format on */
}
