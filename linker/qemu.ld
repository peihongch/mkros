OUTPUT_ARCH(riscv)
ENTRY(_entry)

BASE_ADDRESS = 0x80200000;

SECTIONS
{
    /* Load the kernel at this address: "." means the current address */
    . = BASE_ADDRESS;

    kernel_start = .;

    . = ALIGN(4K);

    text_start = .;
    .text : {
        PROVIDE(stext = .);
        *(.text .text.*)
        . = ALIGN(0x1000);
        _trampoline = .;
        *(trampsec)
        . = ALIGN(0x1000);
        ASSERT(. - _trampoline == 0x1000, "error: trampoline larger than one page");
        PROVIDE(etext = .);
    }

    . = ALIGN(4K);
    rodata_start = .;
    .rodata : {
        PROVIDE(srodata = .);
        . = ALIGN(16);
        *(.rodata .rodata.*)
        PROVIDE(erodata = .);
    }

    . = ALIGN(4K);
    data_start = .;
    .data : {
        PROVIDE(sdata = .);
        . = ALIGN(16);
        *(.data .data.*)
        PROVIDE(edata = .);
    }

    . = ALIGN(4K);
    bss_start = .;
    .bss : {
        PROVIDE(sbss = .);
        . = ALIGN(16);
	    *(.bss.stack)
        sbss_clear = .;
        . = ALIGN(16);
        *(.sbss .bss .bss.*)
        ebss_clear = .;
        PROVIDE(ebss = .);
    }

    . = ALIGN(4K);
    PROVIDE(end = .);
}
