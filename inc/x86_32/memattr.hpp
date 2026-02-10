/*
 * Memory Attributes: x86_32
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

#include "macros.hpp"

// i486 has no PAT; use PCD/PWT bits in PTE for basic cache control
// PCD PWT -> Memory Type
//  0   0  -> WB (Write-Back)
//  0   1  -> WT (Write-Through)
//  1   0  -> UC (Uncacheable)
//  1   1  -> UC (Uncacheable)

#define CA_MEM_WB       0       //  PCD=0 PWT=0
#define CA_MEM_WT       1       //  PCD=0 PWT=1
#define CA_MEM_UC       2       //  PCD=1 PWT=0

#ifndef __ASSEMBLER__

#include "std.hpp"
#include "types.hpp"

class Memattr final
{
    private:
        uint32_t val;

    public:
        static inline constinit unsigned obits { 32 };
        static constexpr unsigned kimax { 0 };
        static constexpr unsigned kbits { 0 };

        // Cacheability, encoded in [1:0]
        enum class Cache : unsigned
        {
            MEM_WB  = CA_MEM_WB,
            MEM_WT  = CA_MEM_WT,
            MEM_UC  = CA_MEM_UC,
            UNUSED,
        };

        Memattr() = default;
        explicit constexpr Memattr (uint32_t v) : val { v } {}
        explicit constexpr Memattr (Cache c) : val { std::to_underlying (c) } {}

        static constexpr auto ram() { return Memattr { Cache::MEM_WB }; }
        static constexpr auto dev() { return Memattr { Cache::MEM_UC }; }

        auto cache_s1() const { return BIT_RANGE (1, 0) & val; }

        bool valid() const { return cache_s1() < std::to_underlying (Cache::UNUSED); }
};

#endif
