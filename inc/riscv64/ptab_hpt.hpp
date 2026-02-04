/*
 * Hypervisor Page Table (HPT) for RISC-V
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

#include "memattr.hpp"
#include "ptab_pte.hpp"

/*
 * Hpt: RISC-V Sv39 Page Table Entry
 */
class Hpt final : public Pte<Hpt, uint64_t, uint64_t>
{
    friend class Pte;

    private:
        // RISC-V Sv39 page table entry bits
        enum
        {
            ATTR_P      = BIT64  (0),   // Valid (Present)
            ATTR_R      = BIT64  (1),   // Read
            ATTR_W      = BIT64  (2),   // Write
            ATTR_X      = BIT64  (3),   // Execute
            ATTR_U      = BIT64  (4),   // User
            ATTR_G      = BIT64  (5),   // Global
            ATTR_A      = BIT64  (6),   // Accessed
            ATTR_D      = BIT64  (7),   // Dirty
            ATTR_nL     = ATTR_R | ATTR_X, // Leaf indicator (any R/W/X set = leaf)
        };

    public:
        static constexpr unsigned ibits { 39 };     // Sv39: 39-bit virtual addresses
        static constexpr auto ptab_attr { ATTR_P }; // Non-leaf entries only have V bit

        // Attributes for PTEs referring to leaf pages
        static OAddr page_attr (unsigned, Paging::Permissions p, Memattr)
        {
            return !(p & Paging::API) ? 0 :
                   ATTR_P | ATTR_A | ATTR_D |
                   (p & Paging::R  ? ATTR_R : 0) |
                   (p & Paging::W  ? ATTR_W : 0) |
                   (p & (Paging::XU | Paging::XS) ? ATTR_X : 0) |
                   (p & Paging::U  ? ATTR_U : 0) |
                   (p & Paging::G  ? ATTR_G : 0);
        }

        auto page_pm() const
        {
            return Paging::Permissions (!val ? 0 :
                                       !!(val & ATTR_G) * Paging::G |
                                       !!(val & ATTR_U) * Paging::U |
                                       !!(val & ATTR_X) * Paging::XS |
                                       !!(val & ATTR_W) * Paging::W |
                                       !!(val & ATTR_R) * Paging::R);
        }

        auto page_ma (unsigned) const
        {
            return Memattr::ram();  // RISC-V doesn't have per-page memory attributes in Sv39
        }

        Hpt() = default;
        Hpt (Entry e) : Pte { e } {}

        // TLB invalidation
        static void invalidate();
        static void invalidate (uintptr_t);
        static void invalidate (uintptr_t, unsigned);
};

/*
 * Hptp: RISC-V Page Table Pointer (wrapper around Ptab)
 */
class Hptp final : public Ptab<Hpt, uint64_t, uint64_t>
{
    private:
        static Hptp master;

    public:
        static auto page_size (unsigned o) { return Hpt::page_size (o); }
        static auto offs_mask (unsigned o) { return Hpt::offs_mask (o); }

        explicit Hptp (OAddr v = 0) : Ptab { Hpt { v } } {}

        ALWAYS_INLINE
        static inline Hptp current()
        {
            uintptr_t val;
            asm volatile ("csrr %0, satp" : "=r" (val));
            // Extract PPN from satp (bits 43:0) and convert to physical address
            return Hptp { (val & BIT64_RANGE (43, 0)) << PAGE_BITS };
        }

        ALWAYS_INLINE
        inline void make_current() const
        {
            // satp format for Sv39: mode=8, ASID=0, PPN=root_addr>>12
            uint64_t satp { (8ULL << 60) | (root_addr() >> PAGE_BITS) };
            asm volatile ("csrw satp, %0; sfence.vma" : : "r" (satp) : "memory");
        }

        ALWAYS_INLINE
        static inline auto master_map (IAddr v, OAddr p, unsigned o, Paging::Permissions pm, Memattr ma)
        {
            return master.update (v, p, o, pm, ma);
        }

        ALWAYS_INLINE
        static inline void *map (IAddr v, OAddr p, Paging::Permissions pm = Paging::Permissions (Paging::R))
        {
            master.update (v, p, 0, pm, Memattr::ram());
            Barrier::sfence_vma (v);
            return reinterpret_cast<void *>(v | (p & offs_mask (0)));
        }

        ALWAYS_INLINE
        static inline void *map_tmp (OAddr p, size_t, Paging::Permissions pm, Memattr, unsigned win)
        {
            // Use simple mapping via MMAP_GLB_MAP0/1
            auto const v { win ? MMAP_GLB_MAP1 : MMAP_GLB_MAP0 };
            return map (v, p, pm);
        }

        void invalidate() const
        {
            asm volatile ("sfence.vma" : : : "memory");
        }
};
