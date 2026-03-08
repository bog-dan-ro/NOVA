OUTPUT_ARCH("i386:x86-64")
OUTPUT_FORMAT("elf64-x86-64")
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

    . = ALIGN(16);
    . += 0x4000;
    __stack_top = .;

    /DISCARD/ : {
        *(.comment*)
        *(.note*)
        *(.eh_frame*)
    }
}
