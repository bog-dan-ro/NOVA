/*
 * Object Space
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
 * Copyright (C) 2019 Udo Steinberg, BedRock Systems, Inc.
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
#include "capability.hpp"
#include "compiler.hpp"
#include "space.hpp"
#include "types.hpp"

class Space_obj : public Space
{
    private:
        static unsigned const lev = 2;
        static unsigned const bpl = bit_scan_reverse (PAGE_SIZE / sizeof (Capability));

        Capability *root { nullptr };

        Capability *walk (unsigned long);

    public:
        static uint64 const num = 1ULL << lev * bpl;

        Capability lookup (unsigned long);

        void update (unsigned long, Capability);
        bool insert (unsigned long, Capability);
};
