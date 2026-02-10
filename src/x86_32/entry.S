/*
 * Entry Functions: x86_32
 *
 * Copyright (C) 2019-2025 Udo Steinberg, BlueRock Security, Inc.
 *
 * This file is part of the NOVA microhypervisor.
 *
 * NOVA is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NOVA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 *
 */

#include "arch.hpp"
#include "entry.hpp"
#include "vectors.hpp"

.data
.global                 handlers
handlers:

.text

.macro                  NOGATE
                        .data
                        .long   0
                        .previous
.endm

.macro                  INTRGATE IDT
                        .balign 4, 0x90
1:
                        .data
                        .long   1b + \IDT
                        .previous
.endm

.macro                  EXCEPTION_ERROR VEC, IDT
INTRGATE                \IDT
                        push    (%esp)
                        movl    $\VEC, 4(%esp)
                        jmp     entry_exc_err
.endm

.macro                  EXCEPTION VEC, IDT
INTRGATE                \IDT
                        push    $\VEC
                        jmp     entry_exc
.endm

.macro                  INTERRUPT VEC
INTRGATE                0
                        push    $\VEC
                        jmp     entry_int
.endm

.macro                  SAVE_STATE
                        SAVE_GPR
                        mov     %esp, %ebx
                        testb   $3, OFS_CS(%ebx)
                        jz      1f
                        cld
                        lea     DSTK_TOP, %esp
1:
.endm

/*
 * Exceptions
 */
EXCEPTION               EXC_DE,     0               // Fault
EXCEPTION               EXC_DB,     0               // Trap or Fault
EXCEPTION               EXC_NMI,    0               // NMI
EXCEPTION               EXC_BP,     IDT_USER        // Trap
EXCEPTION               EXC_OF,     IDT_USER        // Trap
EXCEPTION               EXC_BR,     0               // Fault
EXCEPTION               EXC_UD,     0               // Fault
EXCEPTION               EXC_NM,     0               // Fault
EXCEPTION_ERROR         EXC_DF,     0               // Abort
NOGATE                                              // Abort
EXCEPTION_ERROR         EXC_TS,     0               // Fault
EXCEPTION_ERROR         EXC_NP,     0               // Fault
EXCEPTION_ERROR         EXC_SS,     0               // Fault
EXCEPTION_ERROR         EXC_GP,     0               // Fault
EXCEPTION_ERROR         EXC_PF,     0               // Fault
NOGATE
EXCEPTION               EXC_MF,     0               // Fault
EXCEPTION_ERROR         EXC_AC,     0               // Fault

/*
 * Unused Exceptions
 */
.rept                   NUM_EXC - 0x12
NOGATE
.endr

/*
 * Exception Entry
 */
entry_exc:              push    $0                      // ERR
entry_exc_err:          SAVE_STATE                      // GPR
                        mov     %ebx, %eax
                        push    %eax                    // ARG1 = Exc_regs
                        call    exc_handler
                        add     $4, %esp
exit:                   testb   $3, OFS_CS(%ebx)
                        mov     current, %eax
                        jnz     ret_user_iret
                        LOAD_GPR
                        IRET

/*
 * Interrupt Entry
 */
entry_int:              push    $0                      // ERR
                        SAVE_STATE                      // GPR
                        push    OFS_VEC(%ebx)
                        call    int_handler
                        add     $4, %esp
                        jmp     exit

/*
 * Interrupts
 */
.set                    VEC,    NUM_EXC
.rept                   NUM_VEC-NUM_EXC
INTERRUPT               VEC
.set                    VEC,    VEC + 1
.endr

/*
 * System-Call Entry (via int 0x80)
 */
.balign                 4, 0x90
.global                 entry_sys
entry_sys:
                        push    $0                      // ERR
                        push    $0                      // VEC
                        SAVE_GPR
                        mov     %edi, %esi              // Syscall number from edi
                        mov     current, %eax
                        push    %eax                    // ARG_1 = Ec::current
                        and     $0xf, %esi
                        lea     DSTK_TOP - 4, %esp
                        jmp     *syscall(, %esi, 4)
