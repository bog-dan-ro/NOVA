/*
 * Cache Maintenance: x86_32
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
        static inline void init (unsigned s)
        {
            dcache_line_size = icache_line_size = s;
        }

        // WBINVD is available on i486
        ALWAYS_INLINE
        static inline void data_clean()
        {
            asm volatile ("wbinvd" : : : "memory");
        }

        // i486 has no CLFLUSH; fall back to WBINVD
        ALWAYS_INLINE
        static inline void data_clean (void const *)
        {
            asm volatile ("wbinvd" : : : "memory");
        }

        ALWAYS_INLINE
        static inline void data_clean (void const *, size_t)
        {
            asm volatile ("wbinvd" : : : "memory");
        }
};
