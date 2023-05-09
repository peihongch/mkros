#include "defs.h"
#include "memlayout.h"
#include "param.h"
#include "proc.h"
#include "riscv.h"
#include "spinlock.h"
#include "types.h"

uint64_t sys_exit(void) {
    int n;
    argint(0, &n);
    exit(n);
    return 0;  // not reached
}

uint64_t sys_getpid(void) {
    return myproc()->pid;
}

uint64_t sys_fork(void) {
    return fork();
}

uint64_t sys_wait(void) {
    uint64_t p;
    argaddr(0, &p);
    return wait(p);
}

uint64_t sys_sbrk(void) {
    uint64_t addr;
    int n;

    argint(0, &n);
    addr = myproc()->sz;
    if (growproc(n) < 0)
        return -1;
    return addr;
}

uint64_t sys_sleep(void) {
    int n;
    uint ticks0;

    argint(0, &n);
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n) {
        if (killed(myproc())) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

uint64_t sys_kill(void) {
    int pid;

    argint(0, &pid);
    return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64_t sys_uptime(void) {
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}
