/*
 * Floating Point Unit (FPU): x86_32
 *
 * i486 uses FSAVE/FRSTOR only (no SSE/XSAVE)
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

#include "ec.hpp"
#include "fpu.hpp"
#include "stdio.hpp"

bool Fpu::compact { false };
uint64_t Fpu::hst_xss { 0 };

Slab_cache *Fpu::cache { nullptr };

void Fpu::init()
{
    asm volatile ("fninit");

    trace (TRACE_FPU, "FPU: x87 only (FSAVE/FRSTOR)");
}
