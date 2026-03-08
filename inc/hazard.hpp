/*
 * Hazard
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

#include "atomic.hpp"
#include "macros.hpp"

/*
 * Per-CPU bitmask of pending deferred actions checked at scheduling boundaries
 * (syscall return, IPI, and budget expiry).
 */
class Hazard final
{
    private:
        using hazard_t = unsigned;

        /*
         * CPU A (switching to EC X)            CPU B (setting hazard for EC X)
         *
         * (1) ST.SEQ_CST (Ec::current = X)     (3) ST.SEQ_CST (X->hazard = RECALL)
         * (2) LD.SEQ_CST (X->hazard)           (4) LD.SEQ_CST (Ec::current)
         *
         * Required Memory Ordering
         *
         * (1) sequenced-before (2)             (3) sequenced-before (4)
         * (2) synchronizes-with (3)            (4) synchronizes-with (1)
         */
        Atomic<hazard_t, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST> val;

    public:
        enum
        {
            SCHED       = BIT  (0),     // Reschedule at next safe point
            SLEEP       = BIT  (1),     // Halt CPU after next scheduling decision
            RCU         = BIT  (2),     // Report quiescent state to advance RCU epoch
            TR          = BIT (15),     // (x86) Reload TSS
            FPU         = BIT (16),     // FPU state needs lazy save/restore on next EC switch
            TSC         = BIT (29),     // (x86) Update TSC offset register for new EC
            RECALL      = BIT (30),     // vCPU recall IPI received; preempt guest
            ILLEGAL     = BIT (31),     // Guest executed illegal/unsupported instruction
        };

        explicit constexpr Hazard (hazard_t h) : val { h } {}

        operator hazard_t() const { return val; }

        void set (hazard_t h) { val |=  h; }
        void clr (hazard_t h) { val &= ~h; }
        auto tas (hazard_t h) { return val.test_and_set (h); }
};
