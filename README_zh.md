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
