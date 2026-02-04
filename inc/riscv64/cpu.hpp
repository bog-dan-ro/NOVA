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

#pragma once

#include "arch.hpp"
#include "atomic.hpp"
#include "compiler.hpp"
#include "kmem.hpp"
#include "spinlock.hpp"
#include "std.hpp"

class Cpu final
{
    private:
        static uint64_t ptab                CPULOCAL;           // HS-mode Page Table Root
        static uint64_t hartid              CPULOCAL;           // Hart ID

        static uint64_t mvendorid           CPULOCAL;           // Machine Vendor ID
        static uint64_t marchid             CPULOCAL;           // Machine Architecture ID
        static uint64_t mimpid              CPULOCAL;           // Machine Implementation ID
        static uint64_t misa                CPULOCAL;           // Machine ISA Register

        static Spinlock boot_lock;

        static void enumerate_features();

    public:
        static cpu_t        id              CPULOCAL;
        static unsigned     hazard          CPULOCAL;
        static cpu_t        count;
        static cpu_t        boot_cpu;
        static Atomic<uint32_t> online;

        static bool         feature_h;      // Hypervisor extension

        static uint64_t remote_ptab (cpu_t c)  { return *Kmem::loc_to_glb (c, &ptab); }
        static uint64_t remote_hartid (cpu_t c) { return *Kmem::loc_to_glb (c, &hartid); }

        static void init (cpu_t, unsigned);
        static void init_bsp();
        static void init_ap();

        static auto hpt_base()
        {
            return ptab;
        }

        ALWAYS_INLINE
        static unsigned affinity()
        {
            return static_cast<unsigned>(hartid);
        }

        ALWAYS_INLINE
        static void halt()
        {
            asm volatile ("wfi" : : : "memory");
        }

        ALWAYS_INLINE
        static void preempt_disable()
        {
            asm volatile ("csrc sstatus, %0" : : "r" (SSTATUS_SIE) : "memory");
        }

        ALWAYS_INLINE
        static void preempt_enable()
        {
            asm volatile ("csrs sstatus, %0" : : "r" (SSTATUS_SIE) : "memory");
        }

        // Aliases for syscall.cpp
        ALWAYS_INLINE static void preemption_disable() { preempt_disable(); }
        ALWAYS_INLINE static void preemption_enable() { preempt_enable(); }

        // Enable interrupts briefly to allow preemption, then disable
        ALWAYS_INLINE
        static void preemption_point()
        {
            asm volatile ("csrs sstatus, %0; nop; csrc sstatus, %0" : : "r" (SSTATUS_SIE) : "memory");
        }

        // Power management stub (RISC-V has no ACPI, just halt)
        [[noreturn]]
        static void fini()
        {
            for (;;)
                halt();
        }

        [[noreturn]]
        static void reboot_or_shutdown();
};

