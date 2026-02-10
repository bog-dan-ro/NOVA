/*
 * Ticket Spinlock: x86_32
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
        void lock()
        {
            uint32_t tkt { BIT (16) }, cur;

            asm volatile ("lock xadd    %k1, %0     ;"
                          "     movzwl  %w1, %2     ;"
                          "     shr     $16, %1     ;"
                          "1:   cmp     %k1, %2     ;"
                          "     je      2f          ;"
                          "     rep; nop            ;"
                          "     movzwl  %w0, %2     ;"
                          "     jmp     1b          ;"
                          "2:                       ;"
                          : "+m" (val), "+r" (tkt), "=&r" (cur) : : "memory", "cc");
        }

        ALWAYS_INLINE
        void unlock()
        {
            asm volatile ("incw %0" : "+m" (val) : : "memory", "cc");
        }

        Spinlock() = default;

        Spinlock            (Spinlock const &) = delete;
        Spinlock& operator= (Spinlock const &) = delete;
};
