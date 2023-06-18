#include "kernel.h"
#include "memlayout.h"
#include "param.h"
#include "proc.h"
#include "riscv.h"
#include "sbi.h"
#include "spinlock.h"
#include "types.h"

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

// set up to take exceptions and traps while in the kernel.
void trapinithart(void) {
    w_stvec((uint64_t)kernelvec);
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    // enable supervisor-mode timer interrupts.
    w_sie(r_sie() | SIE_SEIE | SIE_SSIE | SIE_STIE);
    set_next_timeout();
    pr_info("trapinithart");
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void usertrap(void) {
    int which_dev = 0;

    if ((r_sstatus() & SSTATUS_SPP) != 0)
        panic("usertrap: not from user mode");

    // send interrupts and exceptions to kerneltrap(),
    // since we're now in the kernel.
    w_stvec((uint64_t)kernelvec);

    struct proc* p = this_proc();

    // save user program counter.
    p->trapframe->epc = r_sepc();

    if (r_scause() == 8) {
        // system call

        if (killed(p))
            exit(-1);

        // sepc points to the ecall instruction,
        // but we want to return to the next instruction.
        p->trapframe->epc += 4;

        // an interrupt will change sepc, scause, and sstatus,
        // so enable only now that we're done with those registers.
        intr_on();

        syscall();
    } else if ((which_dev = devintr()) != 0) {
        // ok
    } else {
        printk("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
        printk("            sepc=%p stval=%p\n", r_sepc(), r_stval());
        setkilled(p);
    }

    if (killed(p))
        exit(-1);

    // give up the CPU if this is a timer interrupt.
    if (which_dev == 2)
        yield();

    usertrapret();
}

//
// return to user space
//
void usertrapret(void) {
    struct proc* p = this_proc();

    // we're about to switch the destination of traps from
    // kerneltrap() to usertrap(), so turn off interrupts until
    // we're back in user space, where usertrap() is correct.
    intr_off();

    // send syscalls, interrupts, and exceptions to uservec in trampoline.S
    uint64_t trampoline_uservec = TRAMPOLINE + (uservec - trampoline);
    w_stvec(trampoline_uservec);

    // set up trapframe values that uservec will need when
    // the process next traps into the kernel.
    p->trapframe->kernel_satp = r_satp();          // kernel page table
    p->trapframe->kernel_sp = p->kstack + PGSIZE;  // process's kernel stack
    p->trapframe->kernel_trap = (uint64_t)usertrap;
    p->trapframe->kernel_hartid = r_tp();  // hartid for cpuid()

    // set up the registers that trampoline.S's sret will use
    // to get to user space.

    // set S Previous Privilege mode to User.
    unsigned long x = r_sstatus();
    x &= ~SSTATUS_SPP;  // clear SPP to 0 for user mode
    x |= SSTATUS_SPIE;  // enable interrupts in user mode
    w_sstatus(x);

    // set S Exception Program Counter to the saved user pc.
    w_sepc(p->trapframe->epc);

    // tell trampoline.S the user page table to switch to.
    uint64_t satp = MAKE_SATP(p->pagetable);

    // jump to userret in trampoline.S at the top of memory, which
    // switches to the user page table, restores user registers,
    // and switches to user mode with sret.
    uint64_t trampoline_userret = TRAMPOLINE + (userret - trampoline);
    ((void (*)(uint64_t))trampoline_userret)(satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void kerneltrap() {
    int which_dev = 0;
    uint64_t sepc = r_sepc();
    uint64_t sstatus = r_sstatus();
    uint64_t scause = r_scause();

    if ((sstatus & SSTATUS_SPP) == 0)
        panic("kerneltrap: not from supervisor mode");
    if (intr_get() != 0)
        panic("kerneltrap: interrupts enabled");

    if ((which_dev = devintr()) == 0) {
        printk("scause %p\n", scause);
        printk("sepc=%p stval=%p\n", r_sepc(), r_stval());
        panic("kerneltrap");
    }

    // give up the CPU if this is a timer interrupt.
    if (which_dev == 2 && this_proc() != 0 && this_proc()->state == RUNNING)
        yield();

    // the yield() may have caused some traps to occur,
    // so restore trap registers for use by kernelvec.S's sepc instruction.
    w_sepc(sepc);
    w_sstatus(sstatus);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 3 if software interrupt
// 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int devintr() {
    uint64_t scause = r_scause();

    if (scause & SCAUSE_INTERRUPT) {
        switch (scause) {
            case SCAUSE_SSI:
                // software interrupt
                return 3;

            case SCAUSE_STI:
                timer_tick();
                return 2;

            case SCAUSE_SEI:
                // this is a supervisor external interrupt, via PLIC.

                // irq indicates which device interrupted.
                int irq = plic_claim();

                if (irq == UART0_IRQ) {
                    // keyboard input
                    int c = sbi_ecall_console_getc();
                    if (-1 != c) {
                        console_intr(c);
                    }
                } else if (irq == VIRTIO0_IRQ) {
                    virtio_disk_intr();
                } else if (irq) {
                    printk("unexpected interrupt irq=%d\n", irq);
                }

                // the PLIC allows each device to raise at most one
                // interrupt at a time; tell the PLIC the device is
                // now allowed to interrupt again.
                if (irq)
                    plic_complete(irq);

                return 1;

            default:
                return 0;
        }
    }

    return 0;
}
