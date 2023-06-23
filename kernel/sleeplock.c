// Sleeping locks

#include "sleeplock.h"
#include "kernel.h"
#include "memlayout.h"
#include "param.h"
#include "proc.h"
#include "riscv.h"
#include "spinlock.h"
#include "types.h"

void init_sleeplock(struct sleeplock* lk, char* name) {
    initlock(&lk->lk, "sleep lock");
    lk->name = name;
    lk->locked = 0;
    lk->pid = 0;
}

void acquire_sleep(struct sleeplock* lk) {
    acquire(&lk->lk);
    while (lk->locked) {
        sleep(lk, &lk->lk);
    }
    lk->locked = 1;
    lk->pid = this_proc()->pid;
    release(&lk->lk);
}

void release_sleep(struct sleeplock* lk) {
    acquire(&lk->lk);
    lk->locked = 0;
    lk->pid = 0;
    wakeup(lk);
    release(&lk->lk);
}

int holding_sleep(struct sleeplock* lk) {
    int r;

    acquire(&lk->lk);
    r = lk->locked && (lk->pid == this_proc()->pid);
    release(&lk->lk);
    return r;
}
