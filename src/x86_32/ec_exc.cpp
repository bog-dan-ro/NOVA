/*
 * Execution Context Exception Handling: x86_32
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

#include "counter.hpp"
#include "ec_arch.hpp"
#include "fpu.hpp"
#include "gdt.hpp"
#include "nmi.hpp"

void Ec::fpu_load()
{
    assert (fpu);

    fpu->load();

    regs.hazard.set (Hazard::FPU);
}

void Ec::fpu_save()
{
    assert (fpu);

    fpu->save();

    regs.hazard.clr (Hazard::FPU);
}

bool Ec_arch::handle_exc_gp (Exc_regs *)
{
    if (Cpu::hazard & Hazard::TR) {
        Cpu::hazard &= ~Hazard::TR;
        Tss::load();
        return true;
    }

    return false;
}

bool Ec_arch::handle_exc_pf (Exc_regs *r)
{
    auto const pfa { regs.cr2 = Cr::get_cr2() };
    auto const hst { regs.get_hst() };

    if (r->err & BIT (2))       // User-mode access
        return pfa < Space_hst::selectors << PAGE_BITS && hst->loc[Cpu::id].share_from (hst->hptp, pfa, Space_hst::selectors << PAGE_BITS);

    // Kernel fault in PIO space
    if (pfa >= MMAP_SPC_PIO && pfa <= MMAP_SPC_PIO_E && hst->loc[Cpu::id].share_from (hst->hptp, pfa, MMAP_CPU))
        return true;

    // Convert #PF in I/O bitmap to #GP(0)
    if (r->user() && pfa >= MMAP_SPC_PIO && pfa <= MMAP_SPC_PIO_E) {
        r->vec = EXC_GP;
        r->err = regs.cr2 = 0;
        send_msg<ret_user_exception> (this);
    }

    return false;
}

void Ec_arch::handle_exc (Exc_regs *r)
{
    Ec *const self { current };

    switch (r->vec) {

        case EXC_NM:
            if (switch_fpu (self))
                return;
            break;

        case EXC_GP:
            if (static_cast<Ec_arch *>(self)->handle_exc_gp (r))
                return;
            break;

        case EXC_PF:
            if (static_cast<Ec_arch *>(self)->handle_exc_pf (r))
                return;
            break;
    }

    if (r->user()) [[likely]]
        send_msg<ret_user_exception> (self);

    panic ("Fatal exception %#lx (%#lx) at CS:%#lx IP:%#lx", r->vec, r->err, r->cs, r->eip);
}
