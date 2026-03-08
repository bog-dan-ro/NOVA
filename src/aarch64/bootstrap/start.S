.section .text.init
.global _start

_start:
    /* Disable interrupts */
    msr daifset, #0xf

    /* Set up stack (PC-relative) */
    adr x0, __stack_top
    mov sp, x0

    /* Clear BSS */
    adr x0, __bss_start
    adr x1, __bss_end
1:
    cmp x0, x1
    b.ge 2f
    str xzr, [x0], #8
    b 1b
2:
    /* Jump to C code */
    bl start

    /* Hang */
3:
    wfe
    b 3b
