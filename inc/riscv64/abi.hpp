/*
 * Application Binary Interface: Architecture-Specific (RISC-V)
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

#include "regs.hpp"

/*
 * RISC-V Calling Convention:
 * a0-a7 (x10-x17): argument/return registers
 *
 * In our gpr array (x1-x31), indices are:
 * x10 = gpr[9]  (a0)
 * x11 = gpr[10] (a1)
 * x12 = gpr[11] (a2)
 * x13 = gpr[12] (a3)
 * x14 = gpr[13] (a4)
 */
class Sys_abi
{
    private:
        Sys_regs &s;

    public:
        Sys_abi (Sys_regs &r) : s { r } {}

        // a0-a4 (x10-x14) are at gpr indices 9-13
        auto       &p0() const { return s.gpr[9]; }   // a0/x10
        auto       &p1() const { return s.gpr[10]; }  // a1/x11
        auto       &p2() const { return s.gpr[11]; }  // a2/x12
        auto const &p3() const { return s.gpr[12]; }  // a3/x13
        auto const &p4() const { return s.gpr[13]; }  // a4/x14

        ALWAYS_INLINE uint8_t flags() const { return p0() >> 4 & BIT_RANGE (3, 0); }
};
