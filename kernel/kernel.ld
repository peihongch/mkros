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
    *(.text .text.*)
    PROVIDE(etext = .);
  }

  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
  }

  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    *(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
  }

  PROVIDE(end = .);
}
