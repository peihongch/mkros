#ifndef __SLEEPLOCK_H_
#define __SLEEPLOCK_H_

#include "spinlock.h"

// Long-term locks for processes
struct sleeplock {
    uint locked;         // Is the lock held?
    struct spinlock lk;  // spinlock protecting this sleep lock

    // For debugging:
    char* name;  // Name of lock.
    int pid;     // Process holding lock
};

/* clang-format off */

// sleeplock.c
void 	acquire_sleep(struct sleeplock*);
void 	release_sleep(struct sleeplock*);
int 	holding_sleep(struct sleeplock*);
void 	init_sleeplock(struct sleeplock*, char*);

/* clang-format on */

#endif  // __SLEEPLOCK_H_