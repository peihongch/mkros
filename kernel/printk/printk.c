/*
 * copy and then modify from `linux/kernel/printk.c`
 *
 *  (C) 1991  Linus Torvalds
 */

//
// formatted console output -- printk, panic.
//

#include <stdarg.h>
#include <stddef.h>

#include "file.h"
#include "fs.h"
#include "kernel.h"
#include "memlayout.h"
#include "param.h"
#include "proc.h"
#include "riscv.h"
#include "sleeplock.h"
#include "spinlock.h"
#include "types.h"

volatile int panicked = 0;

// lock to avoid interleaving concurrent printk's.
static struct {
    struct spinlock lock;
    int locking;
} pr;

static char buf[1024];

extern int vsprintf(char* buf, const char* fmt, va_list args);

/*
 * When in kernel-mode, we cannot use printf, as fs is liable to
 * point to 'interesting' things. Make a printf with fs-saving, and
 * all is well.
 */
int printk(const char* fmt, ...) {
    va_list args;
    int i, locking;

    locking = pr.locking;
    if (locking)
        acquire(&pr.lock);

    if (fmt == 0)
        panic("null fmt");

    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    console_print(buf);

    if (locking)
        release(&pr.lock);

    return i;
}

void panic(const char* s) {
    pr.locking = 0;
    printk("panic: ");
    printk(s);
    printk("\n");
    panicked = 1;  // freeze uart output from other CPUs
    for (;;)
        ;
}

void printk_init(void) {
    initlock(&pr.lock, "pr");
    pr.locking = 1;
}
