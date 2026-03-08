.section .text.init
.global __init_bsp

__init_bsp:
    csrw sie, zero
    lla sp, __stack_top
    lla t0, __bss_start
    lla t1, __bss_end
bss_clear_loop:
    bgeu t0, t1, bss_clear_done
    sd zero, 0(t0)
    addi t0, t0, 8
    j bss_clear_loop

bss_clear_done:
    j start

hang:
    wfi
    j hang
