K=kernel
BUILD=build

OBJS = \
  $K/entry.o \
  $K/device_tree.o \
  $K/console.o \
  $K/printk.o \
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
  $K/virtio_disk.o

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

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

LDFLAGS = -z max-page-size=4096

ASFLAGS += -I$(CURDIR)/include

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
  $(BUILD) *.dts *.dtb

ifndef CPUS
CPUS := 8
endif

QEMUOPTS = -m 128M -nographic
# use multi-core 
QEMUOPTS += -smp $(CPUS)
# use numa
QEMUOPTS += -numa node,nodeid=0,cpus=0-1,mem=32M \
            -numa node,nodeid=1,cpus=2-3,mem=32M \
            -numa node,nodeid=2,cpus=4-5,mem=32M \
            -numa node,nodeid=3,cpus=6-7,mem=32M
# use opensbi bootloader (fw_dynamic.bin)
QEMUOPTS += -bios $(OPENSBI)

QEMURUN = -machine virt -kernel $(BUILD)/kernel
QEMUDUMPDTS = -machine virt,dumpdtb=virt.dtb
	
run: build
	$(QEMU) $(QEMURUN) $(QEMUOPTS)

dts: 
	$(QEMU) $(QEMUDUMPDTS) $(QEMUOPTS)
	dtc -o virt.dts -O dts virt.dtb
