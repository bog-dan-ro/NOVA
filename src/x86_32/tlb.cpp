/*
 * Translation Lookaside Buffer (TLB): x86_32
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

#include "ec.hpp"
#include "space_hst.hpp"
#include "stdio.hpp"
#include "tlb.hpp"

void Tlb::shootdown (Space *s)
{
    // Single CPU: just flush local TLB
    auto const ec { Ec::remote_current (0) };

    if (ec->regs.get_hst() == s)
        Cpu::hazard |= Hazard::SCHED;
}
