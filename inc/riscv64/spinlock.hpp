/*
 * Ticket Spinlock
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
 */

#pragma once

#include "compiler.hpp"
#include "macros.hpp"
#include "types.hpp"

class Spinlock final
{
    private:
        uint32_t val { 0 };     // nxt[31:16] cur[15:0]

    public:
        ALWAYS_INLINE
        inline void lock()
        {
            uint32_t tmp, ticket;

            // Atomically fetch our ticket using amoswap based approach
            asm volatile ("1:   lr.w.aq  %0, %2         ;"  // tmp = *val (load-reserved, acquire)
                          "     lui      %1, 1          ;"  // %1 = 0x1000 << 4 = 65536
                          "     add      %1, %0, %1     ;"  // %1 = tmp + 65536 (increment nxt)
                          "     sc.w     %1, %1, %2     ;"  // Store-conditional
                          "     bnez     %1, 1b         ;"  // Retry if failed
                          "     srli     %1, %0, 16     ;"  // ticket = tmp >> 16 (our ticket number)
                          "2:   slli     %0, %0, 16     ;"  // Extract cur (shift left then right)
                          "     srli     %0, %0, 16     ;"  // cur = tmp & 0xffff
                          "     beq      %0, %1, 3f     ;"  // if cur == ticket, enter critical section
                          "     lw       %0, %2         ;"  // Reload val
                          "     j        2b             ;"  // Retry
                          "3:   fence    r, rw          ;"  // Acquire barrier
                          : "=&r" (tmp), "=&r" (ticket), "+A" (val)
                          :
                          : "memory");
        }

        ALWAYS_INLINE
        inline void unlock()
        {
            asm volatile ("fence rw, w" ::: "memory");  // Release barrier

            uint32_t tmp;
            asm volatile ("lw    %0, %1   ;"
                          "addi  %0, %0, 1;"
                          "sw    %0, %1   ;"
                          : "=&r" (tmp), "+m" (val)
                          :
                          : "memory");
        }

        Spinlock() = default;

        Spinlock            (Spinlock const &) = delete;
        Spinlock& operator= (Spinlock const &) = delete;
};
