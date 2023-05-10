// Mutual exclusion spin locks.

#include "spinlock.h"
#include "defs.h"
#include "memlayout.h"
#include "param.h"
#include "proc.h"
#include "riscv.h"
#include "types.h"

void initlock(struct spinlock* lk, char* name) {
    lk->name = name;
    lk->locked = 0;  // indicating that the lock is initially not held
    lk->cpu = 0;     // indicating that no processor currently holds the lock
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void acquire(struct spinlock* lk) {
    push_off();  // disable interrupts to avoid deadlock.
    if (holding(lk))
        panic("acquire");

    // _sync_lock_test_and_set() is a built-in function that 
    // provides an atomic memory operation to set a lock and 
    // return the previous value of the lock atomically.
    // 
    // On RISC-V, sync_lock_test_and_set turns into an atomic swap:
    //   a5 = 1
    //   s1 = &lk->locked
    //   amoswap.w.aq a5, a5, (s1)
    while (__sync_lock_test_and_set(&lk->locked, 1) != 0)
        ;

    // Tell the C compiler and the processor to not move loads or stores
    // past this point, to ensure that the critical section's memory
    // references happen strictly after the lock is acquired.
    // On RISC-V, this emits a fence instruction.
    __sync_synchronize();

    // Record info about lock acquisition for holding() and debugging.
    lk->cpu = this_cpu();
}

// Release the lock.
void release(struct spinlock* lk) {
    if (!holding(lk))
        panic("release");

    lk->cpu = 0;

    // Tell the C compiler and the CPU to not move loads or stores
    // past this point, to ensure that all the stores in the critical
    // section are visible to other CPUs before the lock is released,
    // and that loads in the critical section occur strictly before
    // the lock is released.
    // On RISC-V, this emits a fence instruction.
    __sync_synchronize();

    // Release the lock, equivalent to lk->locked = 0.
    // This code doesn't use a C assignment, since the C standard
    // implies that an assignment might be implemented with
    // multiple store instructions.
    // On RISC-V, sync_lock_release turns into an atomic swap:
    //   s1 = &lk->locked
    //   amoswap.w zero, zero, (s1)
    __sync_lock_release(&lk->locked);

    pop_off();
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int holding(struct spinlock* lk) {
    return (lk->locked && lk->cpu == this_cpu());
}

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void push_off(void) {
    int old = intr_get();

    intr_off();
    if (this_cpu()->noff == 0)
        this_cpu()->intena = old;
    this_cpu()->noff += 1;
}

void pop_off(void) {
    struct cpu* c = this_cpu();
    if (intr_get())
        panic("pop_off - interruptible");
    if (c->noff < 1)
        panic("pop_off");
    c->noff -= 1;
    if (c->noff == 0 && c->intena)
        intr_on();
}
