/*
 * Host Memory Space: x86_32
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
#include "cpuset.hpp"
#include "ptab_hpt.hpp"
#include "space_mem.hpp"
#include "tlb.hpp"

class Space_hst final : public Space_mem<Space_hst>
{
    private:
        Space_hst();

        Space_hst (Refptr<Pd> &ref_pd) : Space_mem { Kobject::Subtype::HST, ref_pd } {}

        void collect() override final
        {
            trace (TRACE_DESTROY, "KOBJ: HST %p collected", static_cast<void *>(this));
        }

    public:
        Hptp        hptp;

        Hptp        loc[NUM_CPU];
        Cpuset      cpus;
        Cpuset      htlb;

        static Space_hst nova;
        static Space_hst *current CPULOCAL;

        static constexpr uint8_t  sbw       { Hpt::ibits - PAGE_BITS - 1 };
        static constexpr uint64_t selectors { BIT64 (sbw) };

        static auto mco() { return static_cast<uint8_t>(Hpt::lev_ord()); }

        [[nodiscard]] auto get_ptab (unsigned cpu) { return loc[cpu].root_init(); }

        [[nodiscard]] static Space_hst *create (Status &, Pd *);

        void destroy() override final;

        auto lookup (uint64_t v, uint32_t &p, unsigned &o, Memattr &ma) const
        {
            return hptp.lookup (static_cast<uint32_t>(v), p, o, ma);
        }

        auto update (uint64_t v, uint64_t p, unsigned o, Paging::Permissions pm, Memattr ma) { return hptp.update (static_cast<uint32_t>(v), static_cast<uint32_t>(p), o, pm, ma); }

        void sync() { htlb.set(); Tlb::shootdown (this); }

        ALWAYS_INLINE
        inline void make_current()
        {
            if (current == this) [[likely]]
                return;

            current = this;

            loc[Cpu::id].make_current();
        }

        void init (cpu_t);

        static void access_ctrl (uintptr_t phys, size_t size, Paging::Permissions perm) { Space_mem::access_ctrl (nova, static_cast<uint32_t>(phys), size, perm, Memattr::dev()); }
};
