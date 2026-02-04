/*
 * Low-Level Functions
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

static inline void pause()
{
    // RISC-V doesn't have a dedicated pause instruction in the base ISA
    // The Zihintpause extension adds PAUSE (encoded as FENCE with special operands)
    asm volatile ("fence" : : : "memory");
}

[[noreturn]] static inline void shutdown()
{
    for (;;)
        asm volatile ("wfi" : : : "memory");
}

