#include "cpu.h"
#include "param.h"
#include "riscv.h"

struct cpu cpus[NCPU];

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpuid() {
    int id = r_tp();
    return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu* this_cpu(void) {
    int id = cpuid();
    struct cpu* c = &cpus[id];
    return c;
}