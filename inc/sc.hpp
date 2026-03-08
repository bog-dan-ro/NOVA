/*
 * Scheduling Context (SC)
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
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

#include "ec.hpp"

/*
 * A scheduling context (SC) represents one schedulable slot: it binds a
 * static priority, a CPU budget, and a CPU affinity to exactly one EC.
 *
 * The scheduler's ready queue is ordered by `prio`. Each time slice the SC
 * is given `budget` ticks; `left` tracks how many ticks remain in the
 * current slice and is refilled to `budget` when exhausted. `used` is an
 * atomic accumulator of total CPU time consumed by this SC and can be read
 * from any CPU. `cos` is the Class-of-Service identifier for memory
 * bandwidth partitioning (architecture-dependent).
 */
class Sc final : public Kobject, public Queue<Sc>::Element
{
    friend class Scheduler;

    private:
        Refptr<Ec> const    ec;         // Bound EC (also implies Owner PD)
        uint64_t   const    budget;     // Full per-slice time budget in STC ticks
        cpu_t      const    cpu;        // CPU to which this SC is pinned
        cos_t      const    cos;        // Class-of-Service identifier
        uint8_t    const    prio;       // Static priority (0 = lowest, 127 = highest)
        Atomic<uint64_t>    used    { 0 };  // Total CPU time consumed (atomic for cross-CPU reads)
        uint64_t            left    { 0 };  // Ticks remaining in current slice
        uint64_t            last    { 0 };  // Timer reading when this SC was last scheduled in

        Sc (Refptr<Ec> &, cpu_t, uint16_t, uint8_t, cos_t);

        void collect() override final
        {
            trace (TRACE_DESTROY, "KOBJ: SC %p collected", static_cast<void *>(this));
        }

    public:
        [[nodiscard]] static Sc *create (Status &, Ec *, cpu_t, uint16_t, uint8_t, cos_t);

        void destroy() override final;

        Ec *get_ec() const { return ec; }

        uint64_t get_used() const { return used; }
};
