/*
 * Slab Allocator
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

#include "initprio.hpp"
#include "spinlock.hpp"

/*
 * Fixed-size object allocator backed by page-sized slab pages.
 *
 * Slabs are obtained from the buddy allocator and divided into equal-sized
 * buffers. The slab list is maintained in partial-then-full order:
 *   nullptr <- P <-> P <-> ... <-> P <-> F <-> F -> nullptr
 *              ^                   ^
 *            head                curr
 *
 * `curr` always points to the rightmost partial slab (or nullptr when all
 * slabs are full or the cache is empty). Allocation and free are O(1).
 * Thread-safe via an internal spinlock.
 */
class Slab_cache final
{
    private:
        struct Slab;

        uint16_t const  bsz;                    // Buffer size (rounded up to alignment)
        uint16_t const  bps;                    // Buffers per Slab
        Slab *          curr    { nullptr };    // Current (partial) slab; nullptr if all slabs are full
        Slab *          head    { nullptr };    // Head of Slab list (partial slabs precede full slabs)
        Spinlock        lock;                   // Allocator Spinlock

    public:
        [[nodiscard]] void *alloc();

        void free (void *);

        Slab_cache (size_t, size_t);
};
