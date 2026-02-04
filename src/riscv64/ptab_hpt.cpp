/*
 * Hypervisor Page Table (HPT)
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

#include "ptab_hpt.hpp"
#include "cpu.hpp"

// TLB invalidation for RISC-V
void Hpt::invalidate()
{
    asm volatile ("sfence.vma" : : : "memory");
}

void Hpt::invalidate (uintptr_t addr)
{
    asm volatile ("sfence.vma %0, zero" : : "r" (addr) : "memory");
}

void Hpt::invalidate (uintptr_t addr, unsigned asid)
{
    asm volatile ("sfence.vma %0, %1" : : "r" (addr), "r" (asid) : "memory");
}

