/*
 * Interrupt Handling: x86_32
 *
 * PIC-based interrupt handling for i486
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
#include "idt.hpp"
#include "interrupt.hpp"
#include "pic.hpp"
#include "sm.hpp"
#include "timer.hpp"
#include "vectors.hpp"

INIT_PRIORITY (PRIO_LOCAL) Refptr<Sm> Interrupt::sm_table[NUM_GSI];

void Interrupt::setup()
{
    Idt::build();
}

void Interrupt::handle_gsi (unsigned n)
{
    assert (n < NUM_GSI);

    // Atomic load because other CPUs can update the table concurrently
    Sm *const sm { sm_table[n].atomic_load() };

    if (sm) [[likely]]
        sm->up();
}

void Interrupt::handler (unsigned v)
{
    if (v >= VEC_GSI) {

        unsigned gsi { v - VEC_GSI };

        // Timer interrupt (IRQ 0)
        if (gsi == 0)
            Timer::tick();
        else
            handle_gsi (gsi);
    }

    Pic::eoi (v - VEC_GSI);
}

Status Interrupt::assign (bool, Sm *, Dc const *, cpu_t, uint16_t, uint8_t, uintptr_t &, uintptr_t &)
{
    return Status::BAD_FTR;
}

Status Interrupt::configure (Sm *, uint16_t, uint8_t)
{
    return Status::BAD_FTR;
}

void Interrupt::deactivate (Sm *) {}
