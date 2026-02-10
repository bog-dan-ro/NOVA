/*
 * Execution Context (EC): x86_32
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

#include "assert.hpp"
#include "cpu.hpp"
#include "ec_arch.hpp"
#include "entry.hpp"
#include "event.hpp"
#include "fpu.hpp"
#include "hip.hpp"
#include "pd.hpp"
#include "rcu.hpp"
#include "scheduler.hpp"
#include "stdio.hpp"

// Constructor: Kernel Thread
Ec_arch::Ec_arch (Refptr<Space_obj> &ref_obj, Refptr<Space_hst> &ref_hst, Refptr<Space_pio> &ref_pio, cpu_t c, cont_t x) : Ec { ref_obj, ref_hst, ref_pio, c, x } {}

// Constructor: HST EC
Ec_arch::Ec_arch (bool t, Fpu *f, Refptr<Space_obj> &ref_obj, Refptr<Space_hst> &ref_hst, Refptr<Space_pio> &ref_pio, cpu_t c, unsigned long e, uintptr_t sp, uintptr_t hva, void *k) : Ec { t, f, ref_obj, ref_hst, ref_pio, k, c, e, t ? send_msg<ret_user_exception> : nullptr }
{
    auto const obj { regs.get_obj() };
    auto const hst { regs.get_hst() };
    auto const pio { regs.get_pio() };

    assert (obj && hst && pio && k);

    trace (TRACE_CREATE, "EC:%p created (OBJ:%p HST:%p PIO:%p CPU:%u UTCB:%p %c)", static_cast<void *>(this), static_cast<void *>(obj), static_cast<void *>(hst), static_cast<void *>(pio), c, static_cast<void *>(k), subtype == Kobject::Subtype::EC_LOCAL ? 'L' : 'G');

    // Make sure we have a PTAB for this CPU in the PD
    hst->init (cpu);

    // FIXME: Allocation failure
    assert (hst->get_ptab (c));

    (t ? exc_regs().esp : exc_regs().sp()) = sp;
    exc_regs().set_ep (Event::hst_arch + Event::Selector::STARTUP);

    // Map UTCB
    hst->update (hva, Kmem::ptr_to_phys (kpage), 0, Paging::Permissions (Paging::K | Paging::U | Paging::W | Paging::R), Memattr::ram());
}

void Ec_arch::ret_user_hypercall (Ec *const self)
{
    auto &r { self->regs };

    auto const h { (Cpu::hazard ^ r.hazard) & (Hazard::ILLEGAL | Hazard::RECALL | Hazard::FPU | Hazard::RCU | Hazard::SLEEP | Hazard::SCHED) };
    if (h) [[unlikely]]
        self->handle_hazard (h, ret_user_hypercall);

    Rcu::quiet();

    static_cast<Ec_arch *>(self)->redirect_to_iret();

    trace (TRACE_CONT, "EC:%p %s to CS:%#lx IP:%#lx", static_cast<void *>(self), __func__, r.exc.cs, r.exc.eip);

    asm volatile ("lea %0, %%esp;" EXPAND (LOAD_GPR IRET) : : "m" (r.exc) : "memory");

    UNREACHED;
}

void Ec_arch::ret_user_exception (Ec *const self)
{
    auto &r { self->exc_regs() };

    self->regs.hazard.clr (Hazard::RECALL);

    r.efl = (r.efl | RFL_IF | RFL_1) & ~(RFL_NT | RFL_IOPL);

    asm volatile ("lea %0, %%esp; popa; add $8, %%esp; iret" : : "m" (r) : "memory");

    UNREACHED;
}

Ec *Ec::create_gst (Status &s, Pd *, bool, bool, cpu_t, uintptr_t, uintptr_t, uintptr_t)
{
    s = Status::BAD_FTR;
    return nullptr;
}

void Ec::adjust_offset_ticks (uint64_t) {}

void Ec::handle_hazard (unsigned h, cont_t func)
{
    if (h & Hazard::RCU)
        Rcu::quiet();

    if (h & (Hazard::ILLEGAL | Hazard::RECALL | Hazard::SLEEP | Hazard::SCHED)) [[unlikely]] {

        Cpu::preemption_point();

        if (Cpu::hazard & Hazard::SLEEP) {
            cont = func;
            Cpu::fini();
        }

        if (Cpu::hazard & Hazard::SCHED) {
            cont = func;
            Scheduler::schedule();
        }

        if (h & Hazard::ILLEGAL)
            kill ("Illegal execution state");

        if (regs.hazard & Hazard::RECALL) {

            regs.hazard.clr (Hazard::RECALL);

            if (func == Ec_arch::ret_user_hypercall)
                static_cast<Ec_arch *>(this)->redirect_to_iret();

            exc_regs().set_ep (Event::hst_arch + Event::Selector::RECALL);
            send_msg<Ec_arch::ret_user_exception> (this);
        }
    }

    if (h & Hazard::FPU)
        switch_fpu (this);
}
