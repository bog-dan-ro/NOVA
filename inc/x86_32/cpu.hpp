/*
 * Central Processing Unit (CPU): x86_32
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
#include "config.hpp"
#include "extern.hpp"
#include "kmem.hpp"
#include "spinlock.hpp"
#include "types.hpp"

class Cpu final
{
    private:
        // Must agree with enum class Vendor below
        static constexpr char const *vendor_string[] { "Unknown", "GenuineIntel", "AuthenticAMD" };

        static inline constinit Spinlock boot_lock asm ("__boot_lock");

        static void enumerate_features();

    public:
        enum class Vendor : uint8_t
        {
            UNKNOWN,
            INTEL,
            AMD,
        };

        enum class Feature : unsigned
        {
            // EAX=0x1 (EDX)
            FPU                     =  0 * 32 +  0,     // x87 FPU On-Chip
            PSE                     =  0 * 32 +  3,     // Page Size Extensions
        };

        static cpu_t        id              CPULOCAL_HOT;
        static unsigned     hazard          CPULOCAL_HOT;
        static uint32_t     features[1]     CPULOCAL;
        static bool         bsp             CPULOCAL;

        static inline constinit cpu_t           count  { 0 };
        static inline constinit Atomic<cpu_t>   online { 0 };

        static void init();
        [[noreturn]] static void fini();
        [[noreturn]] static void halt();

        static bool feature (Feature f)
        {
            return features[std::to_underlying (f) / 32] & BIT (std::to_underlying (f) % 32);
        }

        static void preemption_disable()    { asm volatile ("cli" : : : "memory"); }
        static void preemption_enable()     { asm volatile ("sti" : : : "memory"); }
        static void preemption_point()      { asm volatile ("sti; nop; cli" : : : "memory"); }

        static void cpuid (unsigned leaf, uint32_t &eax, uint32_t &ebx, uint32_t &ecx, uint32_t &edx)
        {
            asm volatile ("cpuid" : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (leaf));
        }

        static auto find_by_topology (uint32_t)
        {
            return static_cast<cpu_t>(0);
        }
};
