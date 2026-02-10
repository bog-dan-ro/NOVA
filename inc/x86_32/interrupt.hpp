/*
 * Interrupt Handling: x86_32
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

#include "refcnt.hpp"
#include "status.hpp"
#include "types.hpp"
#include "vectors.hpp"

class Dc;
class Sm;

class Interrupt final
{
    private:
        static Refptr<Sm> sm_table[NUM_GSI];

        static void handle_gsi (unsigned);

    public:
        static constexpr uint8_t  num_vec { NUM_GSI };
        static inline constinit uint16_t num_pin { 16 };

        enum Request
        {
            RRQ,
            RKE,
        };

        static void setup();

        static void *get_ptr (iid_t) { return nullptr; }

        static void handler (unsigned) asm ("int_handler");

        static void send_cpu (Request, cpu_t) {}
        static void send_exc (Request) {}

        static Status assign (bool, Sm *, Dc const *, cpu_t, uint16_t, uint8_t, uintptr_t &, uintptr_t &);

        static Status configure (Sm *, uint16_t, uint8_t);

        static void deactivate (Sm *);
};
