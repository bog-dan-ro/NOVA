/*
 * Cache Maintenance
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

#include "compiler.hpp"
#include "types.hpp"

class Cache final
{
    private:
        static unsigned dcache_line_size CPULOCAL;
        static unsigned icache_line_size CPULOCAL;

    public:
        ALWAYS_INLINE
        static inline void init (unsigned d, unsigned i)
        {
            dcache_line_size = d;
            icache_line_size = i;
        }

        ALWAYS_INLINE
        static inline void data_clean()
        {
            // RISC-V doesn't have explicit cache management in base ISA
            // Zicbom extension provides cache block management
            asm volatile ("fence iorw, iorw" : : : "memory");
        }

        ALWAYS_INLINE
        static inline void data_clean (void const *)
        {
            // RISC-V relies on cache coherency or Zicbom extension
            asm volatile ("fence iorw, iorw" : : : "memory");
        }

        ALWAYS_INLINE
        static inline void data_clean (void const *, size_t, size_t = 64)
        {
            // RISC-V relies on cache coherency or Zicbom extension
            asm volatile ("fence iorw, iorw" : : : "memory");
        }

        ALWAYS_INLINE
        static inline void inst_invalidate()
        {
            asm volatile ("fence.i" : : : "memory");
        }
};

