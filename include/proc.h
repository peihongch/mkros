#ifndef __PROC_H_
#define __PROC_H_

#include "cpu.h"
#include "device_tree.h"
#include "file.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"

extern struct cpu cpus[MAX_CPU];

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {
	/*   0 */ uint64_t kernel_satp;	  // kernel page table
	/*   8 */ uint64_t kernel_sp;	  // top of process's kernel stack
	/*  16 */ uint64_t kernel_trap;	  // usertrap()
	/*  24 */ uint64_t epc;			  // saved user program counter
	/*  32 */ uint64_t kernel_hartid; // saved kernel tp
	/*  40 */ uint64_t ra;
	/*  48 */ uint64_t sp;
	/*  56 */ uint64_t gp;
	/*  64 */ uint64_t tp;
	/*  72 */ uint64_t t0;
	/*  80 */ uint64_t t1;
	/*  88 */ uint64_t t2;
	/*  96 */ uint64_t s0;
	/* 104 */ uint64_t s1;
	/* 112 */ uint64_t a0;
	/* 120 */ uint64_t a1;
	/* 128 */ uint64_t a2;
	/* 136 */ uint64_t a3;
	/* 144 */ uint64_t a4;
	/* 152 */ uint64_t a5;
	/* 160 */ uint64_t a6;
	/* 168 */ uint64_t a7;
	/* 176 */ uint64_t s2;
	/* 184 */ uint64_t s3;
	/* 192 */ uint64_t s4;
	/* 200 */ uint64_t s5;
	/* 208 */ uint64_t s6;
	/* 216 */ uint64_t s7;
	/* 224 */ uint64_t s8;
	/* 232 */ uint64_t s9;
	/* 240 */ uint64_t s10;
	/* 248 */ uint64_t s11;
	/* 256 */ uint64_t t3;
	/* 264 */ uint64_t t4;
	/* 272 */ uint64_t t5;
	/* 280 */ uint64_t t6;
};

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
	struct spinlock lock;

	// p->lock must be held when using these:
	enum procstate state;  // Process state
	void		  *chan;   // If non-zero, sleeping on chan
	int			   killed; // If non-zero, have been killed
	int			   xstate; // Exit status to be returned to parent's wait
	int			   pid;	   // Process ID

	// wait_lock must be held when using this:
	struct proc *parent; // Parent process

	// these are private to the process, so p->lock need not be held.
	uint64_t		  kstack;		 // Virtual address of kernel stack
	uint64_t		  sz;			 // Size of process memory (bytes)
	pagetable_t		  pagetable;	 // User page table
	struct trapframe *trapframe;	 // data page for trampoline.S
	struct context	  context;		 // swtch() here to run process
	struct file		*ofile[NOFILE]; // Open files
	struct inode	 *cwd;			 // Current directory
	char			  name[16];		 // Process name (debugging)
};

#endif // __PROC_H_
