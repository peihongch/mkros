#ifndef __CPU_H_
#define __CPU_H_

#include "types.h"

// Saved registers for kernel context switches.
struct context {
    uint64_t ra;
    uint64_t sp;

    // callee-saved
    uint64_t s0;
    uint64_t s1;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
};

// Per-CPU state.
struct cpu {
    struct proc* proc;       // The process running on this cpu, or null.
    struct context context;  // swtch() here to enter scheduler().
    int noff;                // Depth of push_off() nesting.
    int intena;              // Were interrupts enabled before push_off()?
};

#endif  // __CPU_H_