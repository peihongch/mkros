#ifndef __RISCV_H_
#define __RISCV_H_

#ifndef __ASSEMBLER__
#include "types.h"

/* clang-format off */

/**
 * References:
 * - https://dingfen.github.io/risc-v/2020/08/05/riscv-privileged.html
 */

// which hart (core) is this?
static inline uint64_t r_mhartid() {
    uint64_t x;
    asm volatile("csrr %0, mhartid" : "=r"(x));
    return x;
}

// Machine Status Register, mstatus

/**
 * @brief mstatus: 保存全局中断使能，以及许多其他的状态。
 *
 * +--------+-------------+----+----+----+-----+-----+------+
 * | XLEN-1 | XLEN-2 ~ 23 | 22 | 21 | 20 | 19  | 18  |  17  |
 * +----------------------+----+----+----+-----+-----+------+
 * |   SD   |      0      | TSR| TW | TVM| MXR | SUM | MPRV |
 * +--------+-------------+----+----+----+-----+-----+------+
 *
 * +-----------------------------------------------------------------------------------+
 * | 16 15 | 14 13 | 12 11 | 10 9 |  8  |   7  | 6 |   5  |   4  |  3  | 2 |  1
 * |  0  |
 * +-----------------------------------------------------------------------------------+
 * |   XS  |   FS  |  MPP  |   0  | SPP | MPIE | 0 | SPIE | UPIE | MIE | 0 | SIE
 * | UIE |
 * +-----------------------------------------------------------------------------------+
 *
 * 其中，RV32: XLEN=32, RV64: XLEN=64, 各字段含义如下：
 * - UIE (User Interrupt Enable) - 允许或禁止用户级别的中断。
 * - MIE (Machine Interrupt Enable) - 允许或禁止机器级别的中断。
 * - SIE (Supervisor Interrupt Enable) - 允许或禁止监管者级别的中断。
 * - UPIE (User Previous Interrupt Enable) - 记录上一个用户级别中断是否被允许。
 * - MPIE (Machine Previous Interrupt Enable) -
 * 记录上一个机器级别中断是否被允许。
 * - SPIE (Supervisor Previous Interrupt Enable) -
 * 记录上一个监管者级别中断是否被允许。
 * - SPP (Supervisor Previous Privilege) - 记录上一个指令的特权级别。
 * - MPP (Machine Previous Privilege) - 记录上一个机器级别指令的特权级别。
 * - FS (Floating-Point Status) -
 * 记录当前浮点运算单元的模式（如浮点舍入模式等）。
 * - XS (Extension Status) - 标识当前 CPU 支持的 ISA 扩展。
 * - MPRV (Modify Privilege) -
 * 控制对存储器的访问权限，允许访问更高特权级别的存储器。
 * - SUM (Supervisor User Memory access) - 控制监管者访问用户级别的存储器。
 * - MXR (Make eXecutable Readable) - 允许指令页面被标记为可读。
 * - TVM (Trap Virtual Memory) - 启用虚拟内存陷阱。
 * - TW (Timeout Wait) - 启用时钟中断陷阱。
 * - TSR (Trap SRET) - 启用 SRET 指令陷阱。
 * - SD (State Dirty) - 标识在上一个特权级别中是否发生了状态转换。
 */

#define MSTATUS_MPP_MASK (3L << 11)  // previous mode.
#define MSTATUS_MPP_M (3L << 11)     // machine-mode
#define MSTATUS_MPP_S (1L << 11)     // supervisor-mode
#define MSTATUS_MPP_U (0L << 11)     // user-mode
#define MSTATUS_MIE (1L << 3)        // machine-mode interrupt enable.

static inline uint64_t r_mstatus() {
    uint64_t x;
    asm volatile("csrr %0, mstatus" : "=r"(x));
    return x;
}

static inline void w_mstatus(uint64_t x) {
    asm volatile("csrw mstatus, %0" : : "r"(x));
}

// Machine Exception Program Counter, holds the instruction address
// to which a return from exception will go.
/**
 * @brief mepc(Machine Exception PC): 指向发生异常的指令。
 *
 * @param x 发生异常的指令的地址。
 */
static inline void w_mepc(uint64_t x) {
    asm volatile("csrw mepc, %0" : : "r"(x));
}

// Supervisor Status Register, sstatus

#define SSTATUS_SPP (1L << 8)   // Previous mode, 1=Supervisor, 0=User
#define SSTATUS_SPIE (1L << 5)  // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE (1L << 4)  // User Previous Interrupt Enable
#define SSTATUS_SIE (1L << 1)   // Supervisor Interrupt Enable
#define SSTATUS_UIE (1L << 0)   // User Interrupt Enable

static inline uint64_t r_sstatus() {
    uint64_t x;
    asm volatile("csrr %0, sstatus" : "=r"(x));
    return x;
}

static inline void w_sstatus(uint64_t x) {
    asm volatile("csrw sstatus, %0" : : "r"(x));
}

// Supervisor Interrupt Pending
static inline uint64_t r_sip() {
    uint64_t x;
    asm volatile("csrr %0, sip" : "=r"(x));
    return x;
}

static inline void w_sip(uint64_t x) {
    asm volatile("csrw sip, %0" : : "r"(x));
}

// Supervisor Interrupt Enable

/**
 * @brief sie（Supervisor Interrupt Enable）：
 * 是一个控制中断使能的标志寄存器，用于控制处理器在特权级别转换时对于不同中断的使能状态。
 * 它是一个 64 位的寄存器，当 sie 的某一位为 1 时，表示对应类型的中断使能，
 * 此时处理器可以响应该类型的中断；否则处理器将不响应该类型的中断。
 */

#define SIE_SEIE (1L << 9)  // external
#define SIE_STIE (1L << 5)  // timer
#define SIE_SSIE (1L << 1)  // software
static inline uint64_t r_sie() {
    uint64_t x;
    asm volatile("csrr %0, sie" : "=r"(x));
    return x;
}

static inline void w_sie(uint64_t x) {
    asm volatile("csrw sie, %0" : : "r"(x));
}

// Machine-mode Interrupt Enable
#define MIE_MEIE (1L << 11)  // external
#define MIE_MTIE (1L << 7)   // timer
#define MIE_MSIE (1L << 3)   // software
static inline uint64_t r_mie() {
    uint64_t x;
    asm volatile("csrr %0, mie" : "=r"(x));
    return x;
}

static inline void w_mie(uint64_t x) {
    asm volatile("csrw mie, %0" : : "r"(x));
}

// supervisor exception program counter, holds the
// instruction address to which a return from
// exception will go.
static inline void w_sepc(uint64_t x) {
    asm volatile("csrw sepc, %0" : : "r"(x));
}

static inline uint64_t r_sepc() {
    uint64_t x;
    asm volatile("csrr %0, sepc" : "=r"(x));
    return x;
}

// Machine Exception Delegation

/**
 * @brief medeleg（Machine Exception Delegation Register）
 *  是 RISC-V 架构中的一个特权级寄存器，
 *  用于控制机器模式下哪些异常可以被代理到当前运行的特权级别。
 *
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * | 15  | 14  | 13  | 12  | 11  | 10  | 9   | 8   |
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * | SA  | LA  | SAMA| IAF | S   | LAF | I   | IMA |
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * | 7   | 6   | 5   | 4   | 3   | 2   | 1   | 0   |
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * | S   | SMA | LA  | PA  | B   | IAF | I   | IMA |
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 *
 * 其中，每个比特位对应一个异常类型：
 * - 1 表示将该类型的异常代理到当前运行的特权级别；
 * - 0 表示不代理该类型的异常。
 * 例如，第 3 号比特位表示指令访问权限错误（Instruction Access
 * Fault）异常是否代理， 如果被设置为
 * 1，则代表将该异常类型代理给当前运行的特权级别； 如果被设置为
 * 0，则不代理该异常类型。
 */

static inline uint64_t r_medeleg() {
    uint64_t x;
    asm volatile("csrr %0, medeleg" : "=r"(x));
    return x;
}

static inline void w_medeleg(uint64_t x) {
    asm volatile("csrw medeleg, %0" : : "r"(x));
}

// Machine Interrupt Delegation

/**
 * @brief mideleg(Machine Interrupt Delegation):
 *  机器中断委托 CSR ，控制将哪些中断委托给 S 模式。
 */

static inline uint64_t r_mideleg() {
    uint64_t x;
    asm volatile("csrr %0, mideleg" : "=r"(x));
    return x;
}

static inline void w_mideleg(uint64_t x) {
    asm volatile("csrw mideleg, %0" : : "r"(x));
}

// Supervisor Trap-Vector Base Address
// low two bits are mode.
static inline void w_stvec(uint64_t x) {
    asm volatile("csrw stvec, %0" : : "r"(x));
}

static inline uint64_t r_stvec() {
    uint64_t x;
    asm volatile("csrr %0, stvec" : "=r"(x));
    return x;
}

// Machine-mode interrupt vector
static inline void w_mtvec(uint64_t x) {
    asm volatile("csrw mtvec, %0" : : "r"(x));
}

// Physical Memory Protection

/**
 * @brief 在 RISC-V 中，PMP（Physical Memory
 * Protection，物理内存保护）是一种硬件机制，
 *  用于保护系统中的物理内存免受非授权访问和攻击。PMP 机制通过一组特殊的 PMP
 * 寄存器来实现。 这些寄存器定义了一些 PMP 区域，每个 PMP
 * 区域包含了一个连续的物理内存范围，可以分别配置不同的访问权限。
 *
 * RISC-V 中的 PMP 寄存器包括以下 16 个：
 * - PMPADDR0-PMPADDR15：用于存储每个 PMP 区域的基地址。
 * - PMPCFG0-PMPCFG3：用于配置每个 PMP 区域的访问权限、大小等参数。
 * - PMPCOUNTEREN：控制是否允许 PMP 区域计数器计数。
 * 其中，PMPADDR0-PMPADDR15 寄存器存储每个 PMP 区域的基地址，以 4KB 为单位。
 * 每个 PMP 区域的大小和属性由相应的 PMPCFG 寄存器来配置。
 * PMP 区域的大小可以是 4 字节、8 字节、16 字节、32 字节、64 字节、128 字节、
 * 256 字节、512 字节、1KB、2KB、4KB、8KB、16KB、32KB、64KB 或 128KB。
 * PMPCFG 寄存器的配置可以包括读/写/执行权限、可访问性控制、区域属性等。
 *
 * pmpcfg0的各字段如下：
 *
 * +---+-----+-----+---+---+---+
 * | 7 | 6 5 | 4 3 | 2 | 1 | 0 |
 * +---+-----+-----+---+---+---+
 * | L |  0  |  A  | X | W | R |
 * +---+-----+-----+---+---+---+
 *
 * 其中：
 * - R/W/X分别对应读/写/执行权限。
 * - A表示是否启用此PMP（在这里表示是否启用PMP0）。
 * - L锁定了PMP和对应的地址寄存器。
 *
 * 对于RV32，16 个配置寄存器被分配到 4 个 CSR 中：
 *
 * +---------+---------+--------+-------+
 * | 31 ~ 24 | 23 ~ 16 | 15 ~ 8 | 7 ~ 0 |
 * +---------+---------+--------+-------+
 * |   PMP3  |   PMP2  |  PMP1  |  PMP0 | pmpcfg0
 * +---------+---------+--------+-------+
 * |   PMP7  |   PMP6  |  PMP5  |  PMP4 | pmpcfg1
 * +---------+---------+--------+-------+
 * |  PMP11  |  PMP10  |  PMP9  |  PMP8 | pmpcfg2
 * +---------+---------+--------+-------+
 * |  PMP15  |  PMP14  | PMP13  | PMP12 | pmpcfg3
 * +---------+---------+--------+-------+
 *
 * 对于RV64，它们则分配到了两个偶数编号的 CSR 中：
 *
 * +---------+---------+---------+---------+---------+---------+--------+-------+
 * | 53 ~ 56 | 55 ~ 48 | 47 ~ 40 | 39 ~ 32 | 31 ~ 24 | 23 ~ 16 | 15 ~ 8 | 7 ~ 0
 * |
 * +---------+---------+---------+---------+---------+---------+--------+-------+
 * |   PMP7  |   PMP6  |   PMP5  |   PMP4  |   PMP3  |   PMP2  |  PMP1  |  PMP0
 * | pmpcfg0
 * +---------+---------+---------+---------+---------+---------+--------+-------+
 * |  PMP15  |  PMP14  |  PMP13  |  PMP12  |  PMP11  |  PMP10  |  PMP9  |  PMP8
 * | pmpcfg2
 * +---------+---------+---------+---------+---------+---------+--------+-------+
 *
 */

static inline void w_pmpcfg0(uint64_t x) {
    asm volatile("csrw pmpcfg0, %0" : : "r"(x));
}

/**
 * @brief 在 RISC-V 处理器中，pmpaddr0
 * 是一个特殊寄存器，用于管理物理内存保护（PMP）机制。 PMP
 * 机制可以通过硬件实现，用于限制对特定内存区域的访问权限，从而提高系统的安全性和稳定性。
 *  pmpaddr0 寄存器是 PMP 地址寄存器中的第一个，用于存储第一个 PMP
 * 区域的基地址。 具体来说，pmpaddr0 寄存器的值表示第一个 PMP 区域的起始地址，以
 * 4KB 为单位。 PMP 区域是连续的内存段，其大小和数量可以根据系统的需求进行配置。
 */
static inline void w_pmpaddr0(uint64_t x) {
    asm volatile("csrw pmpaddr0, %0" : : "r"(x));
}

// use riscv's sv39 page table scheme.
#define SATP_SV39 (8L << 60)

#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64_t)pagetable) >> 12))

// supervisor address translation and protection;
// holds the address of the page table.

/**
 * @brief satp (Supervisor Address Translation and Protection):
 *  用于设置和控制地址转换和内存保护，是实现虚拟内存和页表机制的关键之一。
 *
 * RV32:
 *
 * +-------------------------+
 * |  31  | 30 ~ 22 | 21 ~ 0 |
 * +-------------------------+
 * | MODE |   ASID  |   PPN  |
 * +-------------------------+
 *
 * RV64:
 *
 * +----------------------------+
 * | 63 ~ 60 | 59 ~ 44 | 43 ~ 0 |
 * +----------------------------+
 * |   MODE  |   ASID  |   PPN  |
 * +----------------------------+
 *
 * satp 寄存器包含以下字段：
 * - MODE: 模式选择字段。它指示使用哪种页表格式进行地址翻译。
 *    在 RISC-V 中，支持 Sv32 和 Sv39 两种页表格式。
 *    当 MODE 字段为 0 时，使用 Sv32 格式进行地址翻译；
 *    当 MODE 字段为 1 时，使用 Sv39 格式进行地址翻译。
 * - ASID: 地址空间 ID 字段。它用于在多个虚拟地址空间之间进行切换。
 *    不同的地址空间可以使用不同的页表，从而实现更好的内存隔离和保护。
 * - PPN: 物理页号字段。它指示用于地址翻译的顶级页表在物理内存中的位置。
 *    该字段的值应该指向一个页表页（page table page）的物理地址。
 *
 * satp 寄存器为 0 时表示禁用地址转换和保护机制。
 * satp 寄存器通常只能被特权级别为 S-mode 或 M-mode 的软件访问。
 * 在 U-mode 下，该寄存器是只读的。当一个程序想要访问一个虚拟地址时，
 * 处理器将使用 satp 寄存器的信息进行地址转换，从而将虚拟地址映射到物理地址。
 * 如果地址转换失败或访问权限不足，处理器将产生一个异常并跳转到相应的异常处理程序。
 */

static inline void w_satp(uint64_t x) {
    asm volatile("csrw satp, %0" : : "r"(x));
}

static inline uint64_t r_satp() {
    uint64_t x;
    asm volatile("csrr %0, satp" : "=r"(x));
    return x;
}

static inline void w_mscratch(uint64_t x) {
    asm volatile("csrw mscratch, %0" : : "r"(x));
}

// Supervisor Trap Cause

// Interrupts
#define SCAUSE_INTERRUPT 0x8000000000000000L
#define SCAUSE_SSI (SCAUSE_INTERRUPT | 0x1L)  // Supervisor software interrupt
#define SCAUSE_STI (SCAUSE_INTERRUPT | 0x5L)  // Supervisor timer interrupt
#define SCAUSE_SEI (SCAUSE_INTERRUPT | 0x9L)  // Supervisor external interrupt

// Exceptions
#define SCAUSE_IAM 0x0L // Instruction address misaligned
#define SCAUSE_IAF 0x1L // Instruction access fault
#define SCAUSE_II 0x2L  // Illegal instruction
#define SCAUSE_BP 0x3L  // Breakpoint
#define SCAUSE_LAM 0x4L // Load address misaligned
#define SCAUSE_LAF 0x5L // Load access fault
#define SCAUSE_SAM 0x6L // Store/AMO address misaligned
#define SCAUSE_SAF 0x7L // Store/AMO access fault
#define SCAUSE_ECU 0x8L // Environment call from U-mode
#define SCAUSE_ECS 0x9L // Environment call from S-mode
#define SCAUSE_IPF 0xCL // Instruction page fault
#define SCAUSE_LPF 0xDL // Load page fault
#define SCAUSE_SPF 0xFL // Store/AMO page fault

static inline uint64_t r_scause() {
    uint64_t x;
    asm volatile("csrr %0, scause" : "=r"(x));
    return x;
}

// Supervisor Trap Value
static inline uint64_t r_stval() {
    uint64_t x;
    asm volatile("csrr %0, stval" : "=r"(x));
    return x;
}

// Machine-mode Counter-Enable
static inline void w_mcounteren(uint64_t x) {
    asm volatile("csrw mcounteren, %0" : : "r"(x));
}

static inline uint64_t r_mcounteren() {
    uint64_t x;
    asm volatile("csrr %0, mcounteren" : "=r"(x));
    return x;
}

// machine-mode cycle counter
static inline uint64_t r_time() {
    uint64_t x;
    asm volatile("csrr %0, time" : "=r"(x));
    return x;
}

// enable device interrupts
static inline void intr_on() {
    w_sstatus(r_sstatus() | SSTATUS_SIE);
}

// disable device interrupts
static inline void intr_off() {
    w_sstatus(r_sstatus() & ~SSTATUS_SIE);
}

// are device interrupts enabled?
static inline int intr_get() {
    uint64_t x = r_sstatus();
    return (x & SSTATUS_SIE) != 0;
}

static inline uint64_t r_sp() {
    uint64_t x;
    asm volatile("mv %0, sp" : "=r"(x));
    return x;
}

// read and write tp, the thread pointer, which xv6 uses to hold
// this core's hartid (core number), the index into cpus[].
static inline uint64_t r_tp() {
    uint64_t x;
    asm volatile("mv %0, tp" : "=r"(x));
    return x;
}

static inline void w_tp(uint64_t x) {
    asm volatile("mv tp, %0" : : "r"(x));
}

static inline uint64_t r_ra() {
    uint64_t x;
    asm volatile("mv %0, ra" : "=r"(x));
    return x;
}

// flush the TLB.
static inline void sfence_vma() {
    // the zero, zero means flush all TLB entries.
    asm volatile("sfence.vma zero, zero");
}

typedef uint64_t pte_t;
typedef uint64_t* pagetable_t;  // 512 PTEs

#endif  // __ASSEMBLER__

#define PGSIZE 4096  // bytes per page
#define PGSHIFT 12   // bits of offset within a page

#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

#define PTE_V (1L << 0)  // valid
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4)  // user can access

// shift a physical address to the right place for a PTE.
#define PA2PTE(pa) ((((uint64_t)pa) >> 12) << 10)

#define PTE2PA(pte) (((pte) >> 10) << 12)

#define PTE_FLAGS(pte) ((pte)&0x3FF)

// extract the three 9-bit page table indices from a virtual address.
#define PXMASK 0x1FF  // 9 bits
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uint64_t)(va)) >> PXSHIFT(level)) & PXMASK)

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by Sv32/Sv39/Sv48/Sv57, 
// to avoid having to sign-extend virtual addresses that have the high bit set.
#ifdef __RISCV_SV32__
#define MAX_VA (1L << (10 + 10 + 12 - 1))
#endif
#ifdef __RISCV_SV39__
#define MAX_VA (1L << (9 + 9 + 9 + 12 - 1))
#endif
#ifdef __RISCV_SV48__
#define MAX_VA (1L << (9 + 9 + 9 + 9 + 12 - 1))
#endif
#ifdef __RISCV_SV57__
#define MAX_VA (1L << (9 + 9 + 9 + 9 + 9 + 12 - 1))
#endif

#endif  // __RISCV_H_
