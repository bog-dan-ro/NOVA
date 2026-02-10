/*
 * Host Page Table (HPT): x86_32
 *
 * 2-level 32-bit paging: PDE -> PTE (no PAE)
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

#include "ptab_pte.hpp"

class Hpt final : public Pte<Hpt, uint32_t, uint32_t>
{
    friend class Pte;

    private:
        enum
        {
            ATTR_P      = BIT  (0),     // Present
            ATTR_W      = BIT  (1),     // Writable
            ATTR_U      = BIT  (2),     // User
            ATTR_PWT    = BIT  (3),     // Page Write-Through
            ATTR_PCD    = BIT  (4),     // Page Cache Disable
            ATTR_A      = BIT  (5),     // Accessed
            ATTR_D      = BIT  (6),     // Dirty
            ATTR_S      = BIT  (7),     // Superpage (4M, in PDE only)
            ATTR_G      = BIT  (8),     // Global
            ATTR_K      = BIT  (9),     // Kernel Memory (software bit)
        };

    public:
        static constexpr unsigned ibits { 32 };
        static constexpr auto ptab_attr { ATTR_A | ATTR_U | ATTR_W | ATTR_P };

        // Attributes for PTEs referring to leaf pages
        static OAddr page_attr (unsigned l, Paging::Permissions p, Memattr a)
        {
            auto const cache { a.cache_s1() };

            return !(p & Paging::API) ? 0 :
                     ATTR_D  * !!(p & Paging::W)                |
                     ATTR_G  * !!(p & Paging::G)                |
                     ATTR_K  * !!(p & Paging::K)                |
                     ATTR_U  * !!(p & Paging::U)                |
                     ATTR_W  * !!(p & Paging::W)                |
                     ATTR_S  * !!l | ATTR_A | ATTR_P            |
                     // PCD = cache bit 1, PWT = cache bit 0
                     (cache & BIT (1)) << 3 | (cache & BIT (0)) << 3;
        }

        auto page_pm() const
        {
            return Paging::Permissions (!val ? 0 :
                                      !!(val & ATTR_G)  *  Paging::G                |
                                      !!(val & ATTR_K)  *  Paging::K                |
                                      !!(val & ATTR_U)  *  Paging::U                |
                                                           (Paging::XS | Paging::XU) |
                                      !!(val & ATTR_W)  *  Paging::W                |
                                      !!(val & ATTR_P)  *  Paging::R);
        }

        auto page_ma (unsigned) const
        {
            return Memattr { Memattr::Cache ((val >> 4 & BIT (1)) | (val >> 3 & BIT (0))) };
        }

        Hpt() = default;
        Hpt (Entry e) : Pte { e } {}
};

class Hptp final : public Ptab<Hpt, uint32_t, uint32_t>
{
    friend class Space_hst;

    private:
        static Hptp master;

    public:
        explicit Hptp (OAddr v = 0) : Ptab { Hpt { v } } {}

        // Copy Constructor
        Hptp (Hptp const &x) : Ptab { static_cast<Hpt>(x.entry) } {}

        // Copy Assignment
        Hptp& operator= (Hptp const &x)
        {
            entry = static_cast<Hpt>(x.entry);

            return *this;
        }

        ALWAYS_INLINE
        static inline Hptp current()
        {
            uintptr_t val;
            asm volatile ("mov %%cr3, %0" : "=r" (val));
            return Hptp { static_cast<uint32_t>(val & Hpt::addr_mask()) };
        }

        ALWAYS_INLINE
        inline void make_current (uintptr_t = 0) const
        {
            asm volatile ("mov %0, %%cr3" : : "r" (root_addr()) : "memory");
        }

        ALWAYS_INLINE
        static inline void invalidate()
        {
            uintptr_t cr3;
            asm volatile ("mov %%cr3, %0; mov %0, %%cr3" : "=&r" (cr3) : : "memory");
        }

        ALWAYS_INLINE
        static inline void invalidate (uintptr_t addr)
        {
            asm volatile ("invlpg %0" : : "m" (*reinterpret_cast<uintptr_t *>(addr)) : "memory");
        }

        ALWAYS_INLINE
        static inline void master_map (IAddr v, OAddr p, unsigned o, Paging::Permissions pm, Memattr ma)
        {
            master.update (v, p, o, pm, ma);
        }

        bool share_from (Hptp, IAddr, IAddr);
        void share_from_master (IAddr, IAddr);

        static void *map (uintptr_t, OAddr, Paging::Permissions = Paging::R, Memattr = Memattr::ram(), unsigned = 1);

        [[nodiscard]] static void *map_tmp (OAddr, size_t, Paging::Permissions, Memattr, unsigned);
};

// Sanity checks: 2-level paging with 10-bit PDE and 10-bit PTE
static_assert (Hpt::lev() == 2);
static_assert (Hpt::lev_bit (0) == 10 && Hpt::lev_bit (1) == 10);
