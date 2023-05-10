#include "defs.h"
#include "memlayout.h"
#include "param.h"
#include "riscv.h"
#include "types.h"

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

void sys_info();

// start() jumps here in supervisor mode on all CPUs.
int main() {
    if (cpuid() == 0) {
        consoleinit();
        printkinit();
        sys_info();
        printk("\n");
        printk("xv6 kernel is booting\n");
        kinit();  // physical page allocator
        printk("xv6 kernel booting done\n");
        printk("\n");
        __sync_synchronize();
        started = 1;
    } else {
        while (started == 0)
            ;
        __sync_synchronize();
        printk("hart %d started\n", cpuid());
    }

    while (1) {
    }

    return 0;
}

void sys_info() {
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
}
