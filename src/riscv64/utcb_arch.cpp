/*
 * User Thread Control Block (UTCB): Architecture-Specific (RISC-V)
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

#include "regs.hpp"
#include "space_obj.hpp"
#include "utcb_arch.hpp"

void Utcb_arch::load (Mtd_arch const m, Cpu_regs const &c)
{
    auto const &e { c.exc };

    if (m & Mtd_arch::Item::GPR) {
        // Copy GPRs x1-x31 from exception context
        for (unsigned i { 0 }; i < 31; i++)
            user.gpr[i] = e.sys.gpr[i];
    }

    if (m & Mtd_arch::Item::EL_SP)
        user.sp = e.sys.gpr[1];  // x2 (sp) is at index 1

    if (m & Mtd_arch::Item::EL_TP)
        user.tp = e.sys.gpr[3];  // x4 (tp) is at index 3

    if (m & Mtd_arch::Item::EL_EPC)
        csr.sepc = e.csr.sepc;

    if (m & Mtd_arch::Item::EL_STATUS)
        csr.sstatus = e.csr.sstatus;

    if (m & Mtd_arch::Item::EL_CAUSE)
        csr.scause = e.csr.scause;

    if (m & Mtd_arch::Item::EL_TVAL)
        csr.stval = e.csr.stval;
}

bool Utcb_arch::save (Mtd_arch const m, Cpu_regs &c, Space_obj const *obj) const
{
    auto &e { c.exc };

    if (m & Mtd_arch::Item::GPR) {
        // Copy GPRs x1-x31 to exception context
        for (unsigned i { 0 }; i < 31; i++)
            e.sys.gpr[i] = user.gpr[i];
    }

    if (m & Mtd_arch::Item::EL_SP)
        e.sys.gpr[1] = user.sp;  // x2 (sp) is at index 1

    if (m & Mtd_arch::Item::EL_TP)
        e.sys.gpr[3] = user.tp;  // x4 (tp) is at index 3

    if (m & Mtd_arch::Item::EL_EPC)
        e.csr.sepc = csr.sepc;

    if (m & Mtd_arch::Item::EL_STATUS)
        e.csr.sstatus = csr.sstatus;

    // Don't allow modifying scause/stval from user space typically
    // (m & Mtd_arch::Item::EL_CAUSE) - usually read-only to user

    if (m & Mtd_arch::Item::SPACES)
        if (!assign_spaces (c, obj))
            return false;

    return true;
}

bool Utcb_arch::assign_spaces (Cpu_regs &, Space_obj const *) const
{
    // No guest support in minimal RISC-V implementation
    return true;
}
