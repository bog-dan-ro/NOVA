/*
 * Central Processing Unit (CPU): x86_32
 *
 * Minimal i486 CPU initialization
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

#include "cache.hpp"
#include "counter.hpp"
#include "fpu.hpp"
#include "gdt.hpp"
#include "idt.hpp"
#include "space_hst.hpp"
#include "stdio.hpp"
#include "timer.hpp"
#include "tss.hpp"

cpu_t Cpu::id;
unsigned Cpu::hazard;
uint32_t Cpu::features[1];
bool Cpu::bsp;

void Cpu::enumerate_features()
{
    uint32_t eax, ebx, ecx, edx;

    // Check if CPUID is supported (toggle EFLAGS.ID)
    uint32_t flg;
    asm volatile ("pushfl; pushfl; pop %0; xor %1, %0; push %0; popfl; pushfl; pop %0; popfl"
                  : "=&r" (flg) : "i" (RFL_ID));

    if (!(flg & RFL_ID))
        return;

    cpuid (0x1, eax, ebx, ecx, edx);

    features[0] = edx;
}

void Cpu::init()
{
    for (auto func { CTORS_L }; func != CTORS_C; (*func++)()) ;

    Gdt::build();
    Tss::build();

    // Initialize exception handling
    Gdt::load();
    Idt::load();
    Tss::load();

    enumerate_features();

    id = 0;

    Space_hst::nova.loc[id] = Hptp::current();

    // Set CR4 with only applicable bits
    auto cr4 { Cr::get_cr4() };
    if (feature (Feature::PSE))
        cr4 |= CR4_PSE;
    Cr::set_cr4 (cr4);

    Fpu::init();

    Timer::init();

    trace (TRACE_CPU, "CORE: i486 (features:%#x)", features[0]);

    boot_lock.unlock();
}

void Cpu::fini()
{
    halt();
}

void Cpu::halt()
{
    for (;;)
        asm volatile ("sti; hlt; cli");
}
