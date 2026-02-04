/*
 * Entry/Exit Functions
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

#pragma once

// Stack frame size: 31 GPRs + 4 CSRs = 35 registers * 8 bytes = 280 bytes
// Round up to 16-byte alignment = 288 bytes
#define FRAME_SIZE          288

#define DECR_STACK          addi    sp, sp, -FRAME_SIZE
#define INCR_STACK          addi    sp, sp, FRAME_SIZE

// Save general-purpose registers (x1-x31, x0 is always zero)
#define SAVE_GPR            sd      x1,   0*8(sp)   ;\
                            sd      x3,   2*8(sp)   ;\
                            sd      x4,   3*8(sp)   ;\
                            sd      x5,   4*8(sp)   ;\
                            sd      x6,   5*8(sp)   ;\
                            sd      x7,   6*8(sp)   ;\
                            sd      x8,   7*8(sp)   ;\
                            sd      x9,   8*8(sp)   ;\
                            sd      x10,  9*8(sp)   ;\
                            sd      x11, 10*8(sp)   ;\
                            sd      x12, 11*8(sp)   ;\
                            sd      x13, 12*8(sp)   ;\
                            sd      x14, 13*8(sp)   ;\
                            sd      x15, 14*8(sp)   ;\
                            sd      x16, 15*8(sp)   ;\
                            sd      x17, 16*8(sp)   ;\
                            sd      x18, 17*8(sp)   ;\
                            sd      x19, 18*8(sp)   ;\
                            sd      x20, 19*8(sp)   ;\
                            sd      x21, 20*8(sp)   ;\
                            sd      x22, 21*8(sp)   ;\
                            sd      x23, 22*8(sp)   ;\
                            sd      x24, 23*8(sp)   ;\
                            sd      x25, 24*8(sp)   ;\
                            sd      x26, 25*8(sp)   ;\
                            sd      x27, 26*8(sp)   ;\
                            sd      x28, 27*8(sp)   ;\
                            sd      x29, 28*8(sp)   ;\
                            sd      x30, 29*8(sp)   ;\
                            sd      x31, 30*8(sp)   ;

// Load general-purpose registers
#define LOAD_GPR            ld      x1,   0*8(sp)   ;\
                            ld      x3,   2*8(sp)   ;\
                            ld      x4,   3*8(sp)   ;\
                            ld      x5,   4*8(sp)   ;\
                            ld      x6,   5*8(sp)   ;\
                            ld      x7,   6*8(sp)   ;\
                            ld      x8,   7*8(sp)   ;\
                            ld      x9,   8*8(sp)   ;\
                            ld      x10,  9*8(sp)   ;\
                            ld      x11, 10*8(sp)   ;\
                            ld      x12, 11*8(sp)   ;\
                            ld      x13, 12*8(sp)   ;\
                            ld      x14, 13*8(sp)   ;\
                            ld      x15, 14*8(sp)   ;\
                            ld      x16, 15*8(sp)   ;\
                            ld      x17, 16*8(sp)   ;\
                            ld      x18, 17*8(sp)   ;\
                            ld      x19, 18*8(sp)   ;\
                            ld      x20, 19*8(sp)   ;\
                            ld      x21, 20*8(sp)   ;\
                            ld      x22, 21*8(sp)   ;\
                            ld      x23, 22*8(sp)   ;\
                            ld      x24, 23*8(sp)   ;\
                            ld      x25, 24*8(sp)   ;\
                            ld      x26, 25*8(sp)   ;\
                            ld      x27, 26*8(sp)   ;\
                            ld      x28, 27*8(sp)   ;\
                            ld      x29, 28*8(sp)   ;\
                            ld      x30, 29*8(sp)   ;\
                            ld      x31, 30*8(sp)   ;

#define SAVE_STATE          SAVE_GPR                 \
                            csrr    t0, sepc        ;\
                            csrr    t1, sstatus     ;\
                            sd      t0, 31*8(sp)    ;\
                            sd      t1, 32*8(sp)    ;

#define SAVE_STATE_EXC      SAVE_GPR                 \
                            csrr    t0, sepc        ;\
                            csrr    t1, sstatus     ;\
                            csrr    t2, scause      ;\
                            csrr    t3, stval       ;\
                            sd      t0, 31*8(sp)    ;\
                            sd      t1, 32*8(sp)    ;\
                            sd      t2, 33*8(sp)    ;\
                            sd      t3, 34*8(sp)    ;

#define LOAD_STATE          ld      t0, 31*8(sp)    ;\
                            ld      t1, 32*8(sp)    ;\
                            csrw    sepc, t0        ;\
                            csrw    sstatus, t1     ;\
                            LOAD_GPR                ;

#define SRET                sret

