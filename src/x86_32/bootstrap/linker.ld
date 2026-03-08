OUTPUT_ARCH("i386")
OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

PHDRS
{
    init PT_LOAD FLAGS (6);
}

SECTIONS
{
    . = 0x100000;

    .text : {
        *(.text.init)
        *(.text*)
    } : init

    .data : {
        . = ALIGN(4);
        *(.data*)
    } : init

    .rodata : { KEEP(*(.rodata*)) }

    .bss : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(16);
    . += 0x4000;
    __stack_top = .;

    /DISCARD/ : {
        *(.comment*)
        *(.note*)
        *(.eh_frame*)
    }
}
