/*
 * Interrupt Handling
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

#include "intid.hpp"
#include "status.hpp"
#include "types.hpp"

template<typename T> class Refptr;

class Dc;
class Sm;

class Interrupt
{
    public:
        enum Request
        {
            RRQ,        // Remote Reschedule Request
            RKE,        // Remote Kernel Entry
        };

        static void init();

        static void handle_irq();
        static void handle_ipi();
        static void handle_timer();

        static void send_ipi (unsigned, unsigned);
        static void send_cpu (Request, cpu_t);
        static void send_exc (Request);

        static void conf (unsigned, bool);

        static void deactivate (unsigned);
        static void deactivate (Sm *) {}

        // Stub for interrupt semaphore lookup - RISC-V minimal has no MSI support
        static Refptr<Sm> *get_ptr (iid_t) { return nullptr; }

        // Stub for interrupt assignment - RISC-V minimal has no MSI support
        static Status assign (bool, Sm *, Dc const *, cpu_t, uint16_t, uint8_t, uintptr_t &, uintptr_t &)
        {
            return Status::BAD_FTR;
        }
};

