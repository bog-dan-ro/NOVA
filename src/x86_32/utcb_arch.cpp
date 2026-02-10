/*
 * User Thread Control Block (UTCB): x86_32
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

#include "config.hpp"
#include "event.hpp"
#include "regs.hpp"
#include "space_obj.hpp"
#include "space_pio.hpp"
#include "utcb_arch.hpp"

void Utcb_arch::load_exc (Mtd_arch const m, Cpu_regs const &c)
{
    auto const &e { c.exc };
    auto const &s { c.exc.sys };

    if (m & Mtd_arch::Item::GPR_0_7) {
        eax = s.eax; ecx = s.ecx; edx = s.edx; ebx = s.ebx;
        esp = e.esp; ebp = s.ebp; esi = s.esi; edi = s.edi;
    }

    if (m & Mtd_arch::Item::RFLAGS)
        efl = e.efl;

    if (m & Mtd_arch::Item::RIP)
        eip = e.eip;

    if (m & Mtd_arch::Item::CR)
        cr2 = c.cr2;
}

bool Utcb_arch::save_exc (Mtd_arch const m, Cpu_regs &c) const
{
    auto &e { c.exc };
    auto &s { c.exc.sys };

    if (m & Mtd_arch::Item::POISON)
        return false;

    if (m & Mtd_arch::Item::GPR_0_7) {
        s.eax = eax; s.ecx = ecx; s.edx = edx; s.ebx = ebx;
        e.esp = esp; s.ebp = ebp; s.esi = esi; s.edi = edi;
    }

    if (m & Mtd_arch::Item::RFLAGS)
        e.efl = (efl & ~(RFL_NT | RFL_IOPL)) | RFL_IF | RFL_1;

    if (m & Mtd_arch::Item::RIP)
        e.eip = eip;

    return true;
}
