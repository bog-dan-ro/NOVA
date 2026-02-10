/*
 * Message Transfer Descriptor (MTD): Architecture-Specific Part (x86_32)
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
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

#include "mtd.hpp"

class Mtd_arch final : public Mtd
{
    public:
        enum Item
        {
            POISON          = BIT  (0),

            GPR_0_7         = BIT  (1),
            RFLAGS          = BIT  (3),
            RIP             = BIT  (4),

            CS_SS           = BIT (10),
            DS_ES           = BIT (11),
            FS_GS           = BIT (12),

            CR              = BIT (18),

            TLB             = BIT (29),
            FPU             = BIT (30),

            SPACES          = BIT (31),
        };

        explicit Mtd_arch (uint32_t v = 0) : Mtd { v } {}
};
