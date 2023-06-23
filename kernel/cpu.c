#include "cpu.h"
#include "device_tree.h"
#include "param.h"
#include "riscv.h"

struct cpu cpus[MAX_CPU];

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpu_id() {
    int id = r_tp();
    return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu* this_cpu(void) {
    int id = cpu_id();
    struct cpu* c = &cpus[id];
    return c;
}
