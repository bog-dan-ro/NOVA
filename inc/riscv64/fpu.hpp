/*
 * Floating Point Unit (FPU)
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

#include "cpu.hpp"
#include "hazard.hpp"
#include "slab.hpp"

class Fpu final
{
    private:
        struct {
            uint64_t    f[32]       { 0 };      // 32 64-bit FP Registers (D extension)
            uint64_t    fcsr        { 0 };      // FP Control and Status Register
        } regs;

    public:
        // FPU context size
        static constexpr size_t size { sizeof (regs) };

        // FPU context alignment
        static constexpr size_t alignment { 8 };

        void load() const
        {
            asm volatile ("fld  f0,   0*8(%0)  ;"
                          "fld  f1,   1*8(%0)  ;"
                          "fld  f2,   2*8(%0)  ;"
                          "fld  f3,   3*8(%0)  ;"
                          "fld  f4,   4*8(%0)  ;"
                          "fld  f5,   5*8(%0)  ;"
                          "fld  f6,   6*8(%0)  ;"
                          "fld  f7,   7*8(%0)  ;"
                          "fld  f8,   8*8(%0)  ;"
                          "fld  f9,   9*8(%0)  ;"
                          "fld  f10, 10*8(%0)  ;"
                          "fld  f11, 11*8(%0)  ;"
                          "fld  f12, 12*8(%0)  ;"
                          "fld  f13, 13*8(%0)  ;"
                          "fld  f14, 14*8(%0)  ;"
                          "fld  f15, 15*8(%0)  ;"
                          "fld  f16, 16*8(%0)  ;"
                          "fld  f17, 17*8(%0)  ;"
                          "fld  f18, 18*8(%0)  ;"
                          "fld  f19, 19*8(%0)  ;"
                          "fld  f20, 20*8(%0)  ;"
                          "fld  f21, 21*8(%0)  ;"
                          "fld  f22, 22*8(%0)  ;"
                          "fld  f23, 23*8(%0)  ;"
                          "fld  f24, 24*8(%0)  ;"
                          "fld  f25, 25*8(%0)  ;"
                          "fld  f26, 26*8(%0)  ;"
                          "fld  f27, 27*8(%0)  ;"
                          "fld  f28, 28*8(%0)  ;"
                          "fld  f29, 29*8(%0)  ;"
                          "fld  f30, 30*8(%0)  ;"
                          "fld  f31, 31*8(%0)  ;"
                          "ld   t0,  32*8(%0)  ;"
                          "fscsr t0            ;"
                          : : "r" (&regs) : "t0", "memory");
        }

        void save()
        {
            asm volatile ("fsd  f0,   0*8(%0)  ;"
                          "fsd  f1,   1*8(%0)  ;"
                          "fsd  f2,   2*8(%0)  ;"
                          "fsd  f3,   3*8(%0)  ;"
                          "fsd  f4,   4*8(%0)  ;"
                          "fsd  f5,   5*8(%0)  ;"
                          "fsd  f6,   6*8(%0)  ;"
                          "fsd  f7,   7*8(%0)  ;"
                          "fsd  f8,   8*8(%0)  ;"
                          "fsd  f9,   9*8(%0)  ;"
                          "fsd  f10, 10*8(%0)  ;"
                          "fsd  f11, 11*8(%0)  ;"
                          "fsd  f12, 12*8(%0)  ;"
                          "fsd  f13, 13*8(%0)  ;"
                          "fsd  f14, 14*8(%0)  ;"
                          "fsd  f15, 15*8(%0)  ;"
                          "fsd  f16, 16*8(%0)  ;"
                          "fsd  f17, 17*8(%0)  ;"
                          "fsd  f18, 18*8(%0)  ;"
                          "fsd  f19, 19*8(%0)  ;"
                          "fsd  f20, 20*8(%0)  ;"
                          "fsd  f21, 21*8(%0)  ;"
                          "fsd  f22, 22*8(%0)  ;"
                          "fsd  f23, 23*8(%0)  ;"
                          "fsd  f24, 24*8(%0)  ;"
                          "fsd  f25, 25*8(%0)  ;"
                          "fsd  f26, 26*8(%0)  ;"
                          "fsd  f27, 27*8(%0)  ;"
                          "fsd  f28, 28*8(%0)  ;"
                          "fsd  f29, 29*8(%0)  ;"
                          "fsd  f30, 30*8(%0)  ;"
                          "fsd  f31, 31*8(%0)  ;"
                          "frcsr t0            ;"
                          "sd   t0,  32*8(%0)  ;"
                          : : "r" (&regs) : "t0", "memory");
        }

        [[nodiscard]] static Fpu *create (Slab_cache &cache, Hazard &hzd)
        {
            auto const fpu { new (cache) Fpu };

            if (fpu) [[likely]]
                hzd.set (Hazard::FPU);

            return fpu;
        }

        void destroy (Slab_cache &cache)
        {
            operator delete (this, cache);
        }

        [[nodiscard]] static void *operator new (size_t, Slab_cache &cache) noexcept
        {
            return cache.alloc();
        }

        static void operator delete (void *ptr, Slab_cache &cache)
        {
            cache.free (ptr);
        }

        static void enable()
        {
            // Enable FPU in sstatus.FS
            asm volatile ("csrs sstatus, %0" : : "r" (BIT_RANGE (14, 13)));
        }

        static void disable()
        {
            // Disable FPU in sstatus.FS
            asm volatile ("csrc sstatus, %0" : : "r" (BIT_RANGE (14, 13)));
        }
};
