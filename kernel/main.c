#include "device_tree.h"
#include "kernel.h"
#include "memlayout.h"
#include "mm.h"
#include "param.h"
#include "riscv.h"
#include "sbi.h"
#include "types.h"

static inline void init_hartid(unsigned long hartid) { w_tp(hartid); }

volatile static int started = 0;

extern void _entry(void);

void sys_info();

// start() jumps here in supervisor mode on all CPUs.
int main(unsigned long hartid, unsigned long dtb_pa) {
	init_hartid(hartid);
	if (hartid == 0) {
		console_init();
		printk_init();
		printk("\n");
		parse_device_tree(dtb_pa);
		sys_info();

		bootmem_init();	 // init bootmem
		kern_vm_init();	 // create kernel page table
		kvm_init_hart(); // turn on paging
		timerinit();	 // init a lock for timer
		pr_info("hart %d init done", hartid);

		for (int i = 1; i < cpu_num(); i++) {
			unsigned long mask = 1 << i;
			sbi_ecall_hart_start(i, _entry, 0);
			sbi_ecall_send_ipi(mask, 0);
		}
		__sync_synchronize();
		started = 1;
	}
	else {
		sbi_ecall_hart_start(0, _entry, 0);
		sbi_ecall_send_ipi(0, 0);
		while (started == 0)
			;
		__sync_synchronize();
		pr_info("hart %d init done", hartid);
	}

	// install kernel trap vector, including interrupt handler
	trapinithart();

	while (1) {}

	return 0;
}

void sys_info() {
	/* clang-format off */

    printk("\n");
    printk("  __  __   _  __  ____     ___    ____     __              ____  ___ ____   ____   __     __\n");
    printk(" |  \\/  | | |/ / |  _ \\   / _ \\  / ___|   / _| ___  _ __  |  _ \\|_ _/ ___| / ___|  \\ \\   / /\n");
    printk(" | |\\/| | | ' /  | |_) | | | | | \\___ \\  | |_ / _ \\| '__| | |_) || |\\___ \\| |   ____\\ \\ / / \n");
    printk(" | |  | | | . \\  |  _ <  | |_| |  ___) | |  _| (_) | |    |  _ < | | ___) | |__|_____\\ V /  \n");
    printk(" |_|  |_| |_|\\_\\ |_| \\_\\  \\___/  |____/  |_|  \\___/|_|    |_| \\_\\___|____/ \\____|     \\_/   \n");
    printk("\n");

	/* clang-format on */

	printk("text section:\t[%p ~ %p]\n", stext, etext);
	printk("rodata section:\t[%p ~ %p]\n", srodata, erodata);
	printk("data section:\t[%p ~ %p]\n", sdata, edata);
	printk("bss section: \t[%p ~ %p]\n", sbss, ebss);
	printk("\n");

	int			 id;
	node_info	  *node;
	cpu_info	 *cpu;
	memory_info *mem;
	distance	 *dist;

	printk("NUMA Node Num:\t%d\n", node_num());
	for_each_node(id, node) {
		int cpu_id;

		printk("NUMA Node[%d]:\t", id);
		for_each_cpu_of_node(cpu_id, node, cpu) {
			if (cpu_id)
				printk(",");
			printk("CPU[%d]", cpu->cpu_id);
		}
		printk("\n");
	}
	printk("Dist Matrix:\t");
	int last_src = 0;
	for_each_distance_entry(id, dist) {
		if (dist->src != last_src) {
			last_src = dist->src;
			printk("\n\t\t");
		}
		printk("%d\t", dist->value);
	}
	printk("\n");

	printk("CPU Num:\t%d\n", cpu_num());
	for_each_cpu(id, cpu) {
		printk("CPU[%d]:\t\tnuma-node(%d) %s %s %s\n", cpu->cpu_id,
			   cpu->numa_node_id, cpu->status, cpu->riscv_isa, cpu->mmu_type);
	}

	printk("RAM Size:\t%dMB\n", ram_size() >> 20);
	for_each_mem(id, mem) {
		printk("RAM[%d]:\t\tnuma-node(%d) [%p ~ %p]\tSize: %dMB\n", id,
			   mem->numa_node_id, mem->base_address,
			   mem->base_address + mem->ram_size, mem->ram_size >> 20);
	}

	printk("\n");
}
