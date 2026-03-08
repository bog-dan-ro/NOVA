.section .text.init
.global _start
.code32

_start:
    cli

    /* Set up stack */
    movl $__stack_top, %esp

    /* Clear BSS */
    movl $__bss_start, %edi
    movl $__bss_end, %ecx
    subl %edi, %ecx
    shrl $2, %ecx
    xorl %eax, %eax
    rep stosl

    /* Jump to C code */
    call start

    /* Hang */
1:
    hlt
    jmp 1b
