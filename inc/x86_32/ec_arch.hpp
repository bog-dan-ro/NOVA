/*
 * Execution Context (EC): Architecture-Specific (x86_32)
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
 * Copyright (C) 2014 Udo Steinberg, FireEye, Inc.
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

#include "ec.hpp"
#include "extern.hpp"
#include "mtd_arch.hpp"
#include "space_hst.hpp"
#include "tss.hpp"
#include "utcb.hpp"

class Ec_arch final : private Ec
{
    friend class Ec;

    private:
        static constexpr auto needs_pio { true };

        // Constructor: Kernel Thread
        Ec_arch (Refptr<Space_obj> &, Refptr<Space_hst> &, Refptr<Space_pio> &, cpu_t, cont_t);

        // Constructor: HST EC
        Ec_arch (bool, Fpu *, Refptr<Space_obj> &, Refptr<Space_hst> &, Refptr<Space_pio> &, cpu_t, unsigned long, uintptr_t, uintptr_t, void *);

        void collect() override final
        {
            trace (TRACE_DESTROY, "KOBJ: EC %p collected", static_cast<void *>(this));
        }

        static void handle_exc (Exc_regs *) asm ("exc_handler");

        bool handle_exc_gp (Exc_regs *);
        bool handle_exc_pf (Exc_regs *);

        ALWAYS_INLINE
        inline void redirect_to_iret()
        {
            exc_regs().esp = exc_regs().sp();
            exc_regs().eip = exc_regs().ip();
        }

        [[noreturn]] static void ret_user_hypercall (Ec *);

        [[noreturn]] static void ret_user_exception (Ec *) asm ("ret_user_iret");

        ALWAYS_INLINE
        inline void state_load (Ec *const self, Mtd_arch mtd)
        {
            assert (cont == ret_user_exception);

            auto state = self->get_utcb()->arch();

            state->load_exc (mtd, cpu_regs());
        }

        ALWAYS_INLINE
        inline bool state_save (Ec *const self, Mtd_arch mtd)
        {
            assert (cont == ret_user_exception);

            auto state = self->get_utcb()->arch();

            return state->save_exc (mtd, cpu_regs());
        }

        [[noreturn]] ALWAYS_INLINE
        inline void make_current()
        {
            Tss::run.esp0 = reinterpret_cast<uintptr_t>(&exc_regs() + 1);

            auto const hst { regs.get_hst() };
            assert (hst);

            hst->make_current();

            // Reset stack
            asm volatile ("lea %0, %%esp" : : "m" (DSTK_TOP) : "memory");

            // Become current EC and invoke continuation
            (*cont)(current = this);

            UNREACHED;
        }
};
