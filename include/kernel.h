#ifndef __DEFS_H_
#define __DEFS_H_

#include "riscv.h"

struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;

// defined by *.ld linker file
extern char stext[];
extern char etext[]; // *.ld sets this to end of kernel code.
extern char srodata[];
extern char erodata[];
extern char sdata[];
extern char edata[];
extern char sbss[];
extern char ebss[];
extern char kernel_start[]; // kernel start address
extern char kernel_end[];	// first address after kernel.
extern char trampoline[];	// trampoline.S

// bio.c
void		binit(void);
struct buf *bread(uint, uint);
void		brelse(struct buf *);
void		bwrite(struct buf *);
void		bpin(struct buf *);
void		bunpin(struct buf *);

// console.c
void console_init(void);
void console_intr(int);
void console_putchar(int);
void console_print(const char *b);

// timer.c
void timerinit();
void set_next_timeout();
void timer_tick();

// exec.c
int exec(char *, char **);

// file.c
struct file *filealloc(void);
void		 fileclose(struct file *);
struct file *filedup(struct file *);
void		 fileinit(void);
int			 fileread(struct file *, uint64_t, int n);
int			 filestat(struct file *, uint64_t addr);
int			 filewrite(struct file *, uint64_t, int n);

// fs.c
void		  fsinit(int);
int			  dirlink(struct inode *, char *, uint);
struct inode *dirlookup(struct inode *, char *, uint *);
struct inode *ialloc(uint, short);
struct inode *idup(struct inode *);
void		  iinit();
void		  ilock(struct inode *);
void		  iput(struct inode *);
void		  iunlock(struct inode *);
void		  iunlockput(struct inode *);
void		  iupdate(struct inode *);
int			  namecmp(const char *, const char *);
struct inode *namei(char *);
struct inode *nameiparent(char *, char *);
int			  readi(struct inode *, int, uint64_t, uint, uint);
void		  stati(struct inode *, struct stat *);
int			  writei(struct inode *, int, uint64_t, uint, uint);
void		  itrunc(struct inode *);

// ramdisk.c
void ramdiskinit(void);
void ramdiskintr(void);
void ramdiskrw(struct buf *);

// kalloc.c
void *kalloc(void);
void  kfree(void *);
void  kinit(void);

// log.c
void initlog(int, struct superblock *);
void log_write(struct buf *);
void begin_op(void);
void end_op(void);

// pipe.c
int	 pipealloc(struct file **, struct file **);
void pipeclose(struct pipe *, int);
int	 piperead(struct pipe *, uint64_t, int);
int	 pipewrite(struct pipe *, uint64_t, int);

// printk.c
int	 printk(const char *fmt, ...);
void panic(const char *) __attribute__((noreturn));
void printk_init(void);

#define pr_info(fmt, ...) \
	printk("[ INFO][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define pr_warn(fmt, ...) \
	printk("[ WARN][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define pr_err(fmt, ...) \
	printk("[ERROR][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// proc.c
int			 cpu_id(void);
void		 exit(int);
int			 fork(void);
int			 growproc(int);
void		 proc_mapstacks(pagetable_t);
pagetable_t	 proc_pagetable(struct proc *);
void		 proc_freepagetable(pagetable_t, uint64_t);
int			 kill(int);
int			 killed(struct proc *);
void		 setkilled(struct proc *);
struct cpu  *this_cpu(void);
struct proc *this_proc();
void		 procinit(void);
void		 scheduler(void) __attribute__((noreturn));
void		 sched(void);
void		 sleep(void *, struct spinlock *);
void		 userinit(void);
int			 wait(uint64_t);
void		 wakeup(void *);
void		 yield(void);
int	 either_copyout(int user_dst, uint64_t dst, void *src, uint64_t len);
int	 either_copyin(void *dst, int user_src, uint64_t src, uint64_t len);
void procdump(void);

// swtch.S
void swtch(struct context *, struct context *);

// string.c
int	  memcmp(const void *, const void *, uint);
void *memmove(void *, const void *, uint);
void *memset(void *, int, uint);
char *safestrcpy(char *, const char *, int);
int	  strlen(const char *);
int	  strncmp(const char *, const char *, uint);
char *strncpy(char *, const char *, int);

// syscall.c
void argint(int, int *);
int	 argstr(int, char *, int);
void argaddr(int, uint64_t *);
int	 fetchstr(uint64_t, char *, int);
int	 fetchaddr(uint64_t, uint64_t *);
void syscall();

// trap.c
extern uint			   ticks;
void				   trapinit(void);
void				   trapinithart(void);
extern struct spinlock tickslock;
void				   usertrapret(void);

// uart.c
void uartinit(void);
void uartintr(void);
void uartputc(int);
void uartputc_sync(int);
int	 uartgetc(void);

// vm.c
void		kern_vm_init(void);
void		kvm_init_hart(void);
void		kvmmap(pagetable_t, uint64_t, uint64_t, uint64_t, int);
int			mappages(pagetable_t, uint64_t, uint64_t, uint64_t, int);
pagetable_t uvmcreate(void);
void		uvmfirst(pagetable_t, uchar *, uint);
uint64_t	uvmalloc(pagetable_t, uint64_t, uint64_t, int);
uint64_t	uvmdealloc(pagetable_t, uint64_t, uint64_t);
int			uvmcopy(pagetable_t, pagetable_t, uint64_t);
void		uvmfree(pagetable_t, uint64_t);
void		uvmunmap(pagetable_t, uint64_t, uint64_t, int);
void		uvmclear(pagetable_t, uint64_t);
pte_t	  *walk(pagetable_t, uint64_t, int);
uint64_t	walkaddr(pagetable_t, uint64_t);
int			copyout(pagetable_t, uint64_t, char *, uint64_t);
int			copyin(pagetable_t, char *, uint64_t, uint64_t);
int			copyinstr(pagetable_t, char *, uint64_t, uint64_t);

// plic.c
void plicinit(void);
void plicinithart(void);
int	 plic_claim(void);
void plic_complete(int);

// virtio_disk.c
void virtio_disk_init(void);
void virtio_disk_rw(struct buf *, int);
void virtio_disk_intr(void);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

#endif // __DEFS_H_
