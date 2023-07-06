K=kernel
MM=mm
BUILD=build

OBJS = \
  $K/entry.o \
  $K/device_tree.o \
  $K/console.o \
	$K/printk/ring_buffer.o \
  $K/printk/vsprintf.o \
  $K/printk/printk.o \
  $K/timer.o \
  $K/uart.o \
  $K/kalloc.o \
  $K/spinlock.o \
  $K/string.o \
  $K/main.o \
  $K/vm.o \
  $K/cpu.o \
  $K/proc.o \
  $K/swtch.o \
  $K/trampoline.o \
  $K/trap.o \
  $K/syscall.o \
  $K/sysproc.o \
  $K/bio.o \
  $K/fs.o \
  $K/log.o \
  $K/sleeplock.o \
  $K/file.o \
  $K/pipe.o \
  $K/exec.o \
  $K/sysfile.o \
  $K/kernelvec.o \
  $K/plic.o \
  $K/virtio_disk.o \
  $(MM)/bootmem.o

LIBFDT = \
  lib/libfdt/fdt.o \
  lib/libfdt/fdt_ro.o \
  lib/libfdt/fdt_rw.o \
  lib/libfdt/fdt_wip.o \
  lib/libfdt/fdt_addresses.o

SBI = \
  lib/sbi/sbi_string.o

# riscv64-unknown-elf- or riscv64-linux-gnu-
# perhaps in /opt/riscv/bin
#TOOLPREFIX = 

# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if riscv64-unknown-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-elf-'; \
	elif riscv64-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-linux-gnu-'; \
	elif riscv64-unknown-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-linux-gnu-'; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find a riscv64 version of GCC/binutils." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

QEMU = qemu-system-riscv64
linker = ./linker/qemu.ld

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

OPENSBI = ./bootloader/opensbi.bin

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I$(CURDIR)/include
CFLAGS += -I$(CURDIR)/lib
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
# Set mmu mode to Sv48 (all available modes: Sv32/Sv39/Sv48/Sv57)
CFLAGS += -D__RISCV_SV48__

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

LDFLAGS = -z max-page-size=4096

ASFLAGS += -I$(CURDIR)/include
ASFLAGS += -D__RISCV_SV48__

all: build

# Compile Kernel
$(BUILD)/kernel: $(OBJS) $(LIBFDT) $(SBI) $(linker)
	@if [ ! -d "./$(BUILD)" ]; then mkdir $(BUILD); fi
	@$(LD) $(LDFLAGS) -T $(linker) -o $(BUILD)/kernel $(OBJS) $(LIBFDT) $(SBI)
	@$(OBJDUMP) -S $(BUILD)/kernel > $(BUILD)/kernel.asm
	@$(OBJDUMP) -t $(BUILD)/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(BUILD)/kernel.sym

$(BUILD)/kernel.bin: $(BUILD)/kernel
	$(OBJCOPY) --strip-all $< -O binary $@

build: $(BUILD)/kernel.bin
  
clean: 
	rm -rf *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*/*.o */*.d */*.asm */*.sym \
  */**/*.o */**/*.d */**/*.asm */**/*.sym \
  $(BUILD) *.dts *.dtb .gdbinit

ifndef CPUS
CPUS := 8
endif

QEMUOPTS = -m 1024M -nographic
# use multi-core 
QEMUOPTS += -smp $(CPUS)
# config numa, cpu and memory
# TODO: add L1/L2/L3 cache?
QEMUOPTS += -numa node,nodeid=0,cpus=0-1,memdev=mem0 \
						-numa node,nodeid=1,cpus=2-3,memdev=mem1 \
						-numa node,nodeid=2,cpus=4-5,memdev=mem2 \
						-numa node,nodeid=3,cpus=6-7,memdev=mem3 \
						-numa dist,src=0,dst=1,val=20 \
						-numa dist,src=0,dst=2,val=30 \
						-numa dist,src=0,dst=3,val=40 \
						-numa dist,src=1,dst=2,val=20 \
						-numa dist,src=1,dst=3,val=30 \
						-numa dist,src=2,dst=3,val=20 \
						-object memory-backend-ram,id=mem0,size=256M \
						-object memory-backend-ram,id=mem1,size=256M \
						-object memory-backend-ram,id=mem2,size=256M \
						-object memory-backend-ram,id=mem3,size=256M
# use opensbi bootloader (fw_dynamic.bin)
QEMUOPTS += -bios $(OPENSBI)

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

QEMURUN = -machine virt -kernel $(BUILD)/kernel.bin
QEMUDBG = -machine virt -kernel $(BUILD)/kernel
QEMUDUMPDTS = -machine virt,dumpdtb=virt.dtb
	
GDB = gdb-multiarch

run: build
	$(QEMU) $(QEMURUN) $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl-riscv
	sed "s/:1234/:$(GDBPORT)/" < $^ > $@

qemu-gdb: $(BUILD)/kernel .gdbinit
	@echo "*** Now run '$(GDB)' in another window." 1>&2
	$(QEMU) $(QEMUDBG) $(QEMUOPTS) -S $(QEMUGDB)

gdb: 
	$(GDB) $(BUILD)/kernel

dts: 
	$(QEMU) $(QEMUDUMPDTS) $(QEMUOPTS)
	dtc -o virt.dts -O dts virt.dtb

# 定义源文件目录
SRC_DIR := .
# 查找所有.h和.c文件
HDRS := $(shell find $(SRC_DIR) -name '*.h')
SRCS := $(shell find $(SRC_DIR) -name '*.c')
# 定义clang-format命令
CLANG_FORMAT := clang-format
# 定义clang-format的参数
CLANG_FORMAT_FLAGS := -style=file
# 定义目标规则
.PHONY: format
format:
	@$(CLANG_FORMAT) $(CLANG_FORMAT_FLAGS) -i $(HDRS) $(SRCS)
