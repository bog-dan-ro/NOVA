/*
 * Page Table Entry (RISC-V)
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

#include "barrier.hpp"
#include "ptab.hpp"

template<typename T, typename I, typename O> class Pte : public Ptab<T, I, O>::Entry
{
    using E = typename Ptab<T, I, O>::Entry;

    public:
        // In RISC-V Sv39, a PTE is a leaf if any of R, W, X bits are set
        // Otherwise it's a pointer to the next level page table
        auto type (unsigned l) const
        {
            (void) l;
            constexpr uint64_t V_BIT { 1 };         // Valid bit
            constexpr uint64_t RWX_MASK { 0xe };    // R | W | X bits

            if (!E::val || !(E::val & V_BIT))
                return E::Type::HOLE;

            // If R, W, or X bits are set, it's a leaf entry
            if (E::val & RWX_MASK)
                return E::Type::LEAF;

            // Otherwise it's a table pointer (non-leaf)
            return E::Type::PTAB;
        }

        static void publish() { Barrier::sfence_vma(); }

        // Physical address size (Sv39 = 56 bits physical)
        static constexpr auto pas (unsigned) { return 56; }
};
