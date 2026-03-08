.section .text.init
.global _start
.code64

_start:
    cli

    /* Set up stack (RIP-relative) */
    lea __stack_top(%rip), %rsp

    /* Clear BSS */
    lea __bss_start(%rip), %rdi
    lea __bss_end(%rip), %rcx
    sub %rdi, %rcx
    shr $3, %rcx
    xor %eax, %eax
    rep stosq

    /* Jump to C code */
    call start

    /* Hang */
1:
    hlt
    jmp 1b
