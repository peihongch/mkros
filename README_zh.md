# mkros

> 状态：早期开发

[English](./README.md) | 简体中文

## 介绍

基于RISC-V架构的微内核操作系统。

| 名称       | 配置                      |
| ---------- | ------------------------- |
| 指令集架构 | RISC-V 64 (rv64imafdc)    |
| 处理器平台 | QEMU Virt                 |
| 引导程序   | OpenSBI v1.2-116-g7919530 |

## 特性

- [x] OpenSBI引导程序
- [x] 多处理器 / 多核心
- [x] 非一致内存访问（NUMA）
- [x] 动态RAM管理
- [ ] 伙伴系统
- [ ] Slab分配器
- [ ] 其他

## 快速上手

通过下面的步骤构建和在 QEMU 平台上运行 mkros：

```
$ make && make run
```
运行输出如下：
```
OpenSBI v1.2-116-g7919530
   ____                    _____ ____ _____
  / __ \                  / ____|  _ \_   _|
 | |  | |_ __   ___ _ __ | (___ | |_) || |
 | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
 | |__| | |_) |  __/ | | |____) | |_) || |_
  \____/| .__/ \___|_| |_|_____/|___/_____|
        | |
        |_|

Platform Name             : riscv-virtio,qemu
Platform Features         : medeleg
Platform HART Count       : 8
Platform IPI Device       : aclint-mswi
Platform Timer Device     : aclint-mtimer @ 10000000Hz
Platform Console Device   : semihosting
Platform HSM Device       : ---
Platform PMU Device       : ---
Platform Reboot Device    : sifive_test
Platform Shutdown Device  : sifive_test
Platform Suspend Device   : ---
Platform CPPC Device      : ---
Firmware Base             : 0x80000000
Firmware Size             : 288 KB
Firmware RW Offset        : 0x20000
Runtime SBI Version       : 1.0

Domain0 Name              : root
Domain0 Boot HART         : 0
Domain0 HARTs             : 0*,1*,2*,3*,4*,5*,6*,7*
Domain0 Region00          : 0x0000000080000000-0x000000008001ffff M: (R,X) S/U: ()
Domain0 Region01          : 0x0000000002000000-0x000000000203ffff M: (I,R,W) S/U: ()
Domain0 Region02          : 0x0000000080000000-0x000000008007ffff M: (R,W) S/U: ()
Domain0 Region03          : 0x0000000000000000-0xffffffffffffffff M: (R,W,X) S/U: (R,W,X)
Domain0 Next Address      : 0x0000000080200000
Domain0 Next Arg1         : 0x0000000087000000
Domain0 Next Mode         : S-mode
Domain0 SysReset          : yes
Domain0 SysSuspend        : yes

Boot HART ID              : 0
Boot HART Domain          : root
Boot HART Priv Version    : v1.10
Boot HART Base ISA        : rv64imafdc
Boot HART ISA Extensions  : time
Boot HART PMP Count       : 16
Boot HART PMP Granularity : 4
Boot HART PMP Address Bits: 54
Boot HART MHPM Count      : 0
Boot HART MIDELEG         : 0x0000000000000222
Boot HART MEDELEG         : 0x000000000000b109

[ WARN][kernel/device_tree.c:284] ignore node reserved-memory
[ WARN][kernel/device_tree.c:284] ignore node fw-cfg@10100000
[ WARN][kernel/device_tree.c:284] ignore node flash@20000000
[ WARN][kernel/device_tree.c:284] ignore node chosen

  __  __   _  __  ____     ___    ____     __              ____  ___ ____   ____   __     __
 |  \/  | | |/ / |  _ \   / _ \  / ___|   / _| ___  _ __  |  _ \|_ _/ ___| / ___|  \ \   / /
 | |\/| | | ' /  | |_) | | | | | \___ \  | |_ / _ \| '__| | |_) || |\___ \| |   ____\ \ / / 
 | |  | | | . \  |  _ <  | |_| |  ___) | |  _| (_) | |    |  _ < | | ___) | |__|_____\ V /  
 |_|  |_| |_|\_\ |_| \_\  \___/  |____/  |_|  \___/|_|    |_| \_\___|____/ \____|     \_/   

text section:	[0x0000000080200000 ~ 0x000000008020c000]
rodata section:	[0x000000008020c000 ~ 0x000000008020cfac]
data section:	[0x000000008020d000 ~ 0x000000008020d034]
bss section: 	[0x000000008020e000 ~ 0x0000000080240850]

Node Num:	4
Node[0]:	CPU[0],CPU[1]
Node[1]:	CPU[2],CPU[3]
Node[2]:	CPU[4],CPU[5]
Node[3]:	CPU[6],CPU[7]
CPU Num:	8
CPU[0]:		numa-node(0) okay rv64imafdcsu riscv,sv48
CPU[1]:		numa-node(0) okay rv64imafdcsu riscv,sv48
CPU[2]:		numa-node(1) okay rv64imafdcsu riscv,sv48
CPU[3]:		numa-node(1) okay rv64imafdcsu riscv,sv48
CPU[4]:		numa-node(2) okay rv64imafdcsu riscv,sv48
CPU[5]:		numa-node(2) okay rv64imafdcsu riscv,sv48
CPU[6]:		numa-node(3) okay rv64imafdcsu riscv,sv48
CPU[7]:		numa-node(3) okay rv64imafdcsu riscv,sv48
RAM Size:	128MB
RAM[0]:		numa-node(0) [0x0000000080000000 ~ 0x0000000082000000]	Size: 32MB
RAM[1]:		numa-node(1) [0x0000000082000000 ~ 0x0000000084000000]	Size: 32MB
RAM[2]:		numa-node(2) [0x0000000084000000 ~ 0x0000000086000000]	Size: 32MB
RAM[3]:		numa-node(3) [0x0000000086000000 ~ 0x0000000088000000]	Size: 32MB

[ INFO][kernel/main.c:37] 
[ INFO][kernel/main.c:38] hart 0 enter main()...
[ INFO][kernel/kalloc.c:48] kernel_end: 0x0000000080241000, phystop: 0x0000000088000000
[ INFO][kernel/kalloc.c:49] kinit
[ INFO][kernel/timer.c:12] timerinit
[ INFO][kernel/trap.c:24] trapinithart
[ INFO][kernel/main.c:43] hart 0 init done
[ INFO][kernel/main.c:44] 
[ INFO][kernel/main.c:53] mkros kernel started
[ INFO][kernel/main.c:60] hart 4 enter main()...
[ INFO][kernel/main.c:60] hart 2 enter main()...
[ INFO][kernel/trap.c:24] trapinithart
[ INFO][kernel/trap.c:24] trapinithart
[ INFO][kernel/main.c:62] hart 2 init done
[ INFO][kernel/main.c:62] hart 4 init done
[ INFO][kernel/main.c:60] hart 7 enter main()...
[ INFO][kernel/trap.c:24] trapinithart
[ INFO][kernel/main.c:62] hart 7 init done
[ INFO][kernel/main.c:60] hart 3 enter main()...
[ INFO][kernel/trap.c:24] trapinithart
[ INFO][kernel/main.c:62] hart 3 init done
[ INFO][kernel/main.c:60] hart 6 enter main()...
[ INFO][kernel/trap.c:24] trapinithart
[ INFO][kernel/main.c:62] hart 6 init done
[ INFO][kernel/main.c:60] hart 5 enter main()...
[ INFO][kernel/main.c:60] hart 1 enter main()...
```

本项目也提供了 [gdb dashboard](https://github.com/cyrus-and/gdb-dashboard) 配置来使得调试更简单。

如果你想调试 mkros, 可以尝试以下的步骤：

1. 打开终端并启动 gdb 调试服务器：

   ```shell
   $ make gdb
   *** Now run 'gdb-multiarch' in another window.
   qemu-system-riscv64 -machine virt -kernel build/kernel -m 1024M -nographic -smp 8 -numa node,nodeid=0,cpus=0-1,memdev=mem0 -numa node,nodeid=1,cpus=2-3,memdev=mem1 -numa node,nodeid=2,cpus=4-5,memdev=mem2 -numa node,nodeid=3,cpus=6-7,memdev=mem3 -numa dist,src=0,dst=1,val=20 -numa dist,src=0,dst=2,val=30 -numa dist,src=0,dst=3,val=40 -numa dist,src=1,dst=2,val=20 -numa dist,src=1,dst=3,val=30 -numa dist,src=2,dst=3,val=20 -object memory-backend-ram,id=mem0,size=256M -object memory-backend-ram,id=mem1,size=256M -object memory-backend-ram,id=mem2,size=256M -object memory-backend-ram,id=mem3,size=256M -bios ./bootloader/opensbi.bin -S -gdb tcp::25000
   ```

2. 然后打开另一个终端并连接到调试服务器（以 gdb-multiarch 为例）：

   ```shell
   $ gdb-multiarch build/kernel
   ```

   接着你就可以看到 gdb dashboard 的界面：

   ```shell
   ─── Assembly ───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   0x0000000000001000  ? auipc	t0,0x0
   0x0000000000001004  ? addi	a2,t0,40
   0x0000000000001008  ? csrr	a0,mhartid
   0x000000000000100c  ? ld	a1,32(t0)
   0x0000000000001010  ? ld	t0,24(t0)
   0x0000000000001014  ? jr	t0
   0x0000000000001018  ? unimp
   0x000000000000101a  ? .2byte	0x8000
   0x000000000000101c  ? unimp
   0x000000000000101e  ? unimp
   ─── Breakpoints ────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   ─── Expressions ────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   ─── History ────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   ─── Memory ─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   ─── Registers ──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
      zero 0x0000000000000000     ra 0x0000000000000000      sp 0x0000000000000000      gp 0x0000000000000000     tp 0x0000000000000000    t0 0x0000000000000000    t1 0x0000000000000000    t2 0x0000000000000000
        fp 0x0000000000000000     s1 0x0000000000000000      a0 0x0000000000000000      a1 0x0000000000000000     a2 0x0000000000000000    a3 0x0000000000000000    a4 0x0000000000000000    a5 0x0000000000000000
        a6 0x0000000000000000     a7 0x0000000000000000      s2 0x0000000000000000      s3 0x0000000000000000     s4 0x0000000000000000    s5 0x0000000000000000    s6 0x0000000000000000    s7 0x0000000000000000
        s8 0x0000000000000000     s9 0x0000000000000000     s10 0x0000000000000000     s11 0x0000000000000000     t3 0x0000000000000000    t4 0x0000000000000000    t5 0x0000000000000000    t6 0x0000000000000000
        pc 0x0000000000001000
   ─── Source ─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   ─── Stack ──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   [0] from 0x0000000000001000
   ─── Threads ────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   [8] id 8 from 0x0000000000001000
   [7] id 7 from 0x0000000000001000
   [6] id 6 from 0x0000000000001000
   [5] id 5 from 0x0000000000001000
   [4] id 4 from 0x0000000000001000
   [3] id 3 from 0x0000000000001000
   [2] id 2 from 0x0000000000001000
   [1] id 1 from 0x0000000000001000
   ─── Variables ──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   ────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
   >>> 
   ```

   现在，你就可以使用 gdb 命令来调试 mkros 了。
