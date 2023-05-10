OUTPUT_ARCH( "riscv" )
ENTRY( _entry )
BASE_ADDRESS = 0x80000000;

SECTIONS
{
  /*
   * ensure that entry.S / _entry is at BASE_ADDRESS,
   * where qemu's -kernel jumps.
   */
  . = BASE_ADDRESS;

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

  .rodata : {
    PROVIDE(srodata = .);
    . = ALIGN(16);
    *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
    PROVIDE(erodata = .);
  }

  .data : {
    PROVIDE(sdata = .);
    . = ALIGN(16);
    *(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    *(.data .data.*)
    PROVIDE(edata = .);
  }

  .bss : {
    PROVIDE(sbss = .);
    . = ALIGN(16);
    *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
    PROVIDE(ebss = .);
  }

  PROVIDE(end = .);
}
