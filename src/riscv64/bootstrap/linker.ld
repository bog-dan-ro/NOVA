OUTPUT_ARCH("riscv")
ENTRY(_start)

ENTRY(__init_bsp)

PHDRS
{
    init PT_LOAD FLAGS (6);
}

SECTIONS
{
    /* The base address here is just a reference for the linker.
       Because we use PC-relative addressing, the actual runtime address won't matter. */
    . = 0x800c0000;

    .text : {
        *(.text.init) /* Ensure our boot assembly is at the very beginning */
        *(.text*)
    } : init

    .sdata : {
        . = ALIGN(8);
        *(.data*)
    } : init

    .rodata : { KEEP(*(.rodata*)) }

    .bss : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /* Allocate 16KB for the stack at the end of the image */
    . = ALIGN(16);
    . += 0x4000;
    __stack_top = .;
}
