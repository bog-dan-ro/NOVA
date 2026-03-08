/*
 * Memory Space
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

#include "bits.hpp"
#include "memattr.hpp"
#include "memory.hpp"
#include "paging.hpp"
#include "space.hpp"

/*
 * Common base for memory spaces (host, guest, DMA).
 *
 * access_ctrl() is a helper that breaks a physically-contiguous region into
 * naturally-aligned page blocks of the largest possible order and calls
 * T::update() on each, applying the given permissions and memory attributes.
 *
 * Subclasses must provide the concrete page-table type T and implement
 * T::update(). The template parameter T is the concrete space type (e.g.
 * Space_hst, Space_gst).
 *
 * delegate() copies a mapping from a Space_hst source into this space.
 */
template<typename T> class Space_mem : public Space
{
    protected:
        Space_mem (Kobject::Subtype s) : Space { s } {}

        Space_mem (Kobject::Subtype s, Refptr<Pd> &ref_pd) : Space { s, ref_pd } {}

        static void access_ctrl (T &mem, uintptr_t phys, size_t size, Paging::Permissions perm, Memattr attr)
        {
            for (unsigned o; size; size -= BITN (o), phys += BITN (o))
                mem.update (phys, phys, (o = aligned_order (size, phys)) - PAGE_BITS, perm, attr);
        }

    public:
        [[nodiscard]] Status delegate (Space_hst const *, unsigned long, unsigned long, unsigned, unsigned, Memattr);
};
