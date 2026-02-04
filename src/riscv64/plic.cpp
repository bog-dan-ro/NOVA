/*
 * PLIC (Platform-Level Interrupt Controller)
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

#include "plic.hpp"
#include "stdio.hpp"

void Plic::init (cpu_t cpu)
{
    trace (TRACE_INTR, "PLIC: Initializing for CPU %u", cpu);

    // Enable supervisor external interrupts for this hart
    // Set threshold to 0 (allow all priorities)
}

void Plic::conf (unsigned irq, bool msk)
{
    // Context 1 is S-mode for hart 0 in QEMU
    unsigned const ctx { 1 };

    if (!msk)
        enable (ctx, irq);
    else
        disable (ctx, irq);
}

void Plic::send_sgi (unsigned, cpu_t)
{
    // RISC-V doesn't have SGIs in PLIC
    // Use IPI via SBI or CLINT
}

