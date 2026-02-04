/*
 * Message Transfer Descriptor (MTD): Architecture-Specific Part (RISC-V)
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
            ICI             = BIT  (1),

            GPR             = BIT  (2),     // General Purpose Registers x1-x31
            FPR             = BIT  (3),     // Floating Point Registers f0-f31

            EL_SP           = BIT  (4),     // Stack Pointer
            EL_TP           = BIT  (5),     // Thread Pointer (tp/x4)

            EL_EPC          = BIT (10),     // Exception PC (sepc)
            EL_STATUS       = BIT (11),     // Status register (sstatus)
            EL_CAUSE        = BIT (12),     // Cause register (scause)
            EL_TVAL         = BIT (13),     // Trap value (stval)

            EL_SATP         = BIT (15),     // S-mode Address Translation and Protection
            EL_SCRATCH      = BIT (16),     // Scratch register

            SPACES          = BIT (31),
        };

        Mtd_arch() : Mtd { 0 } {}
        Mtd_arch (uint32_t v) : Mtd { v } {}
};
