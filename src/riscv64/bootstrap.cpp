/*
 * Bootstrap Code
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

#include "cpu.hpp"
#include "stdio.hpp"

extern "C" [[noreturn]] void bootstrap()
{
    trace (0, "NOVA booting on RISC-V hart %u", Cpu::affinity());

    // Initialize BSP CPU
    Cpu::init_bsp();

    trace (0, "Bootstrap complete");

    // Main scheduler loop
    for (;;)
        Cpu::halt();
}
