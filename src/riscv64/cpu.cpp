/*
 * Central Processing Unit (CPU)
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
#include "console.hpp"
#include "lowlevel.hpp"

uint64_t    Cpu::ptab                CPULOCAL;
uint64_t    Cpu::hartid              CPULOCAL;
uint64_t    Cpu::mvendorid           CPULOCAL;
uint64_t    Cpu::marchid             CPULOCAL;
uint64_t    Cpu::mimpid              CPULOCAL;
uint64_t    Cpu::misa                CPULOCAL;
unsigned    Cpu::hazard              CPULOCAL;
cpu_t       Cpu::id                  CPULOCAL;

cpu_t       Cpu::count;
cpu_t       Cpu::boot_cpu;
Atomic<uint32_t> Cpu::online;

bool        Cpu::feature_h;

Spinlock    Cpu::boot_lock;

void Cpu::enumerate_features()
{
    // Read ISA from device tree or CSR (if available)
    // The misa CSR is only accessible in M-mode
    // In S-mode, we rely on device tree or SBI

    // Check for H extension (hypervisor)
    // This would typically come from device tree parsing
    feature_h = false;
}

void Cpu::init (cpu_t c, unsigned h)
{
    id = c;
    hartid = h;

    enumerate_features();

    Console::print ("CPU:%02u Hart:%u ISA: RV64%s%s%s%s%s%s\n",
                    id, static_cast<unsigned>(hartid),
                    "I",    // Integer base
                    "M",    // Integer multiply/divide
                    "A",    // Atomics
                    "F",    // Single-precision floating-point
                    "D",    // Double-precision floating-point
                    "C");   // Compressed instructions
}

void Cpu::init_bsp()
{
    boot_cpu = 0;
    count = 1;

    init (0, 0);

    online++;
}

void Cpu::init_ap()
{
    online++;
}

void Cpu::reboot_or_shutdown()
{
    // Use SBI to shutdown
    // ECALL to SBI with shutdown extension
    register unsigned long a7 asm ("a7") = 0x08;    // SBI_EXT_SRST
    register unsigned long a6 asm ("a6") = 0x0;     // SBI_SHUTDOWN
    register unsigned long a0 asm ("a0") = 0x0;     // Shutdown type
    register unsigned long a1 asm ("a1") = 0x0;     // Reason

    asm volatile ("ecall" : "+r" (a0) : "r" (a1), "r" (a6), "r" (a7));

    shutdown();
}

