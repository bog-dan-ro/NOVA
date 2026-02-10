/*
 * Floating Point Unit (FPU): x86_32
 *
 * i486 only supports x87 FSAVE/FRSTOR
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
#include "cr.hpp"
#include "hazard.hpp"
#include "slab.hpp"

class Fpu final
{
    private:
        // x87 FPU state saved by FSAVE (108 bytes)
        struct Legacy
        {
            uint32_t    fcw;
            uint32_t    fsw;
            uint32_t    ftw;
            uint32_t    fip;
            uint32_t    fcs;
            uint32_t    fdp;
            uint32_t    fds;
            uint8_t     st[80];         // 8 x87 registers (10 bytes each)
        };

        static_assert (__is_standard_layout (Legacy) && sizeof (Legacy) == 108);

    public:
        struct State_xsv
        {
            Legacy      legacy {};

            static constexpr auto size { sizeof (Legacy) };
        };

        // x87 FPU context size
        static inline constinit size_t size { sizeof (Legacy) };

        // x87 FPU context alignment
        static constexpr size_t alignment { 4 };

        static bool     compact;
        static uint64_t hst_xss;

        // i486 has no XSAVE
        static unsigned context_size() { return sizeof (Legacy); }
        static auto context_full() { return sizeof (Legacy); }

        static Slab_cache *cache;

        [[nodiscard]] static void *operator new (size_t, Slab_cache &c) noexcept { return c.alloc(); }
        static void  operator delete (void *ptr, Slab_cache &c) { c.free (ptr); }

        State_xsv state;

        void save()
        {
            asm volatile ("fsave %0" : "=m" (state.legacy));
        }

        void load()
        {
            asm volatile ("frstor %0" : : "m" (state.legacy));
        }

        static void init();

        static void disable()
        {
            Cr::set_cr0 (Cr::get_cr0() | CR0_TS);
        }

        static void enable()
        {
            asm volatile ("clts");
        }
};
