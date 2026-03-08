/*
 * Timeout
 *
 * Copyright (C) 2014 Udo Steinberg, FireEye, Inc.
 * Copyright (C) 2019-2026 Udo Steinberg, BlueRock Security, Inc.
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

#include "types.hpp"

struct Region
{
        uintptr_t                  addr                              = 0;
        size_t                     size                              = 0;
        constexpr bool             operator== (const Region &) const = default;
        constexpr bool             operator!= (const Region &) const = default;
        constexpr inline uintptr_t end() const { return addr + size; }
        constexpr inline bool      contains (Region reg) const
        {
            return addr <= reg.addr && end() >= reg.end();
        }
        constexpr inline bool intersects (Region reg) const
        {
            return addr < reg.addr + reg.size && reg.addr < addr + size;
        }
};

struct BootInfo
{
        size_t    boot_cpu = 0;
        Region    fdt; // FDT address
        size_t    cpu_count = 1;
        size_t    timer_hz  = 1'000'000;
        Region    plic; // PLIC address
        uintptr_t boot_args = 0;
        Region    ram_regions[0]; // all ram regions, ends with an empty (addr & size = 0) region
};

struct RegionIterator
{
        struct Sentinel {};

        struct Iterator
        {
                Region *ptr;

                constexpr Region   &operator*  ()         const { return *ptr; }
                constexpr Iterator &operator++ ()               { ++ptr; return *this; }
                constexpr bool      operator== (Sentinel)  const { return !ptr->addr && !ptr->size; }
        };

        Region *start;

        constexpr RegionIterator (Region *s) : start { s } {}
        constexpr Iterator begin() const { return Iterator { start }; }
        constexpr Sentinel end()   const { return Sentinel {}; }
};
