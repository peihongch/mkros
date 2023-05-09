#ifndef __SPINLOCK_H_
#define __SPINLOCK_H_

#include "cpu.h"
#include "types.h"

// Mutual exclusion lock.
struct spinlock {
    uint locked;  // Is the lock held?

    // For debugging:
    char* name;       // Name of lock.
    struct cpu* cpu;  // The cpu holding the lock.
};

#endif  // __SPINLOCK_H_
