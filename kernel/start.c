#include "memlayout.h"
#include "param.h"
#include "riscv.h"

int main();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__((aligned(16))) char stack0[4096 * NCPU];

// a scratch area per CPU for machine-mode timer interrupts.
uint64_t timer_scratch[NCPU][5];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// entry.S jumps here in machine mode on stack0.
void start() {
    // set M Previous Privilege mode to Supervisor, for mret.
    unsigned long x = r_mstatus();
    x &= ~MSTATUS_MPP_MASK;
    x |= MSTATUS_MPP_S;
    w_mstatus(x);

    // set M Exception Program Counter to main, for mret.
    // requires gcc -mcmodel=medany
    w_mepc((uint64_t)main);

    // disable paging for now.
    w_satp(0);

    // delegate all interrupts and exceptions to supervisor mode.
    w_medeleg(0xffff);
    w_mideleg(0xffff);
    w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

    // configure Physical Memory Protection to give supervisor mode
    // access to all of physical memory.
    w_pmpaddr0(0x3fffffffffffffull);
    w_pmpcfg0(0xf);

    // ask for clock interrupts.
    timerinit();

    // keep each CPU's hartid in its tp register, for cpuid().
    int id = r_mhartid();
    w_tp(id);

    // switch to supervisor mode and jump to main().
    // mret 将 PC 设置为 mepc，通过将 mstatus 的 MPIE 域复制到 MIE
    // 来恢复之前的中断使能设置， 并将权限模式设置为 mstatus 的 MPP 域中的值。
    asm volatile("mret");
}

// arrange to receive timer interrupts.
// they will arrive in machine mode at at timervec in kernelvec.S,
// which turns them into software interrupts for devintr() in trap.c.
void timerinit() {
    // each CPU has a separate source of timer interrupts.
    int id = r_mhartid();

    // ask the CLINT for a timer interrupt.
    int interval = 1000000;  // cycles; about 1/10th second in qemu.
    /**
     * This ensures that a timer interrupt will be generated
     * after the specified interval has elapsed.
     */
    *(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;

    // prepare information in scratch[] for timervec.
    // scratch[0..2] : space for timervec to save registers.
    // scratch[3] : address of CLINT MTIMECMP register.
    // scratch[4] : desired interval (in cycles) between timer interrupts.
    uint64_t* scratch = &timer_scratch[id][0];
    scratch[3] = CLINT_MTIMECMP(id);
    scratch[4] = interval;
    w_mscratch((uint64_t)scratch);

    // set the machine-mode trap handler.
    w_mtvec((uint64_t)timervec);

    /**
     * - 处理器在机器模式下运行，在全局中断使能位 mstatus.MIE 置 1
     * 时才会产生中断。
     * - 每个中断在控制状态寄存器 mie 以及 mip 中都有自己的使能位。
     * 例如，将所有三个控制状态寄存器合在一起考虑：
     *  如果 mstatus.MIE=1，mie[7]=1，且 mip[7]=1，则 CPU
     * 就会开始处理机器模式的时钟中断。
     */

    // enable machine-mode interrupts.
    w_mstatus(r_mstatus() | MSTATUS_MIE);  // MIE = mstatus.1

    // enable machine-mode timer interrupts.
    w_mie(r_mie() | MIE_MTIE);  // mip[7] = 1
}
