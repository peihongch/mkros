// Physical memory layout

#ifndef __MEM_LAYOUT_H_
#define __MEM_LAYOUT_H_

#include "riscv.h"

// qemu -machine virt is set up like this,
// based on qemu's hw/riscv/virt.c:
//
// 00001000 -- boot ROM, provided by qemu
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0 
// 10001000 -- virtio disk 
// 80000000 -- boot ROM jumps here in machine mode
//             -kernel loads the kernel here
// unused RAM after 80000000.

// the kernel uses physical memory thus:
// 80000000 -- entry.S, then kernel text and data
// end -- start of kernel page allocation area
// PHYSTOP -- end RAM used by the kernel

// qemu puts UART registers here in physical memory.
#define UART0 0x10000000L
#define UART0_IRQ 10

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

// core local interruptor (CLINT), which contains the timer.
/**
 * The CLINT is typically implemented as a memory-mapped device, 
 * with registers accessible via memory-mapped I/O (MMIO). 
 * It provides a set of standard registers, including the machine timer (MTIME) 
 * and the machine timer compare (MTIMECMP) registers, 
 * which are used for generating timer interrupts.
 * 
 * The address 0x2000000L is the base address of the Core Local Interruptor (CLINT) 
 * in a RISC-V system when using the default memory map.
 * 
 * In RISC-V, the memory map is divided into various regions, 
 * with each region assigned a specific range of addresses. 
 * The CLINT is typically located in the memory-mapped I/O (MMIO) region 
 * of the memory map, which starts at address 0x20000000. 
 * The CLINT itself starts at the 0x0 offset within this region, 
 * hence the base address of the CLINT is 0x20000000 + 0x0 = 0x2000000.
 * 
 * It's worth noting that the exact memory map of a RISC-V system 
 * is implementation-dependent and may vary between different RISC-V processors and systems. 
 * However, the 0x2000000L address is a common default address 
 * for the CLINT in many RISC-V implementations.
 */
#define CLINT 0x2000000L
/**
 * The MTIMECMP registers are located at an offset of 0x4000 from the base address, 
 * and each MTIMECMP register is 8 bytes in size.
 */
#define CLINT_MTIMECMP(hartid) (CLINT + 0x4000 + 8*(hartid))
/**
 * The MTIME register is located at an offset of 0xBFF8 bytes from the base address of the CLINT.
 */
#define CLINT_MTIME (CLINT + 0xBFF8) // cycles since boot.

// qemu puts platform-level interrupt controller (PLIC) here.
#define PLIC 0x0c000000L
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)

// map the trampoline page to the highest address,
// in both user and kernel space.
#define TRAMPOLINE (MAX_VA - PGSIZE)

// map kernel stacks beneath the trampoline,
// each surrounded by invalid guard pages.
#define KSTACK(p) (TRAMPOLINE - ((p)+1)* 2*PGSIZE)

// User memory layout.
// Address zero first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   TRAPFRAME (p->trapframe, used by the trampoline)
//   TRAMPOLINE (the same page as in the kernel)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)

#endif // __MEM_LAYOUT_H_
