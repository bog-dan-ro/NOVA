/*
 * Portal (PT)
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
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

#include "ec.hpp"
#include "mtd_arch.hpp"

/*
 * A portal (PT) is an IPC endpoint that binds an entry instruction pointer
 * and a message-transfer descriptor (MTD) to a target execution context.
 *
 * When an EC calls a portal, NOVA copies the UTCB fields indicated by `mtd`,
 * stores `id` in the UTCB, and transfers execution to the bound EC at `ip`.
 * The portal identifier (`id`) and MTD can be updated atomically at any time
 * via set_id() and set_mtd().
 */
class Pt final : public Kobject
{
    private:
        Refptr<Ec> const ec;    // Bound EC (also implies Owner PD)
        uintptr_t  const    ip;     // Entry IP: instruction pointer on portal invocation

        /*
         * Memory Ordering
         *
         * ID/MTD changes are observable as follows:
         * - Ambient CPU: after ctrl_pt returned
         * - Remote CPUs: after external ACQUIRE/RELEASE synchronization with ambient CPU, denoting that ctrl_pt returned
         */
        Atomic<uintptr_t, __ATOMIC_RELAXED, __ATOMIC_RELAXED, __ATOMIC_RELAXED> id  { 0 };              // Portal identifier echoed in UTCB on entry
        Atomic<Mtd_arch,  __ATOMIC_RELAXED, __ATOMIC_RELAXED, __ATOMIC_RELAXED> mtd { Mtd_arch { 0 } }; // Message-transfer descriptor

        explicit Pt (Refptr<Ec> &, uintptr_t);

        void collect() override final;

    public:
        [[nodiscard]] static Pt *create (Status &, Ec *, uintptr_t);

        void destroy() override final;

        Ec *get_ec() const { return ec; }

        uintptr_t get_ip() const { return ip; }

        uintptr_t get_id() const { return id; }

        Mtd_arch get_mtd() const { return mtd; }

        void set_id (uintptr_t i) { id = i; }

        void set_mtd (Mtd_arch m) { mtd = m; }
};
