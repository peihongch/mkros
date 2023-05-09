#include "defs.h"
#include "memlayout.h"
#include "param.h"
#include "riscv.h"
#include "types.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
int main() {
    if (cpuid() == 0) {
        consoleinit();
        printfinit();
        printf("\n");
        printf("xv6 kernel is booting\n");
        printf("\n");
        __sync_synchronize();
        started = 1;
    } else {
        while (started == 0)
            ;
        __sync_synchronize();
        printf("hart %d starting\n", cpuid());
    }

    while (1) {
    }

    return 0;
}