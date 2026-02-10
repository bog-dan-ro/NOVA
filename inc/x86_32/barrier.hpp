/*
 * Barriers: x86_32
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

class Barrier final
{
    public:
        // i486 lacks lfence/sfence/mfence, use lock prefix for full barrier
        static void rmb() { asm volatile ("lock; addl $0, (%%esp)" : : : "memory"); }
        static void wmb() { asm volatile ("lock; addl $0, (%%esp)" : : : "memory"); }
        static void fmb() { asm volatile ("lock; addl $0, (%%esp)" : : : "memory"); }
};
