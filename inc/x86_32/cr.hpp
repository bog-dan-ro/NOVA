/*
 * Control Registers (CR): x86_32
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
#include "types.hpp"

class Cr final
{
    public:
        #define DEFINE_CR(X)                                    \
                                                                \
        ALWAYS_INLINE                                           \
        static auto get_cr##X()                                 \
        {                                                       \
            uintptr_t val;                                      \
            asm volatile ("mov %%cr" #X ", %0" : "=r" (val));   \
            return val;                                         \
        }                                                       \
                                                                \
        ALWAYS_INLINE                                           \
        static void set_cr##X (uintptr_t val)                   \
        {                                                       \
            asm volatile ("mov %0, %%cr" #X : : "r" (val));     \
        }

        DEFINE_CR (0)
        DEFINE_CR (2)
        DEFINE_CR (4)
};
