/*
 * Interrupt Descriptor Table (IDT): x86_32
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

#include "descriptor.hpp"
#include "vectors.hpp"

class Idt final
{
    public:
        static inline constinit ALIGNED(8) Descriptor_idt idt[NUM_VEC];

        static void build();

        static void load()
        {
            Pseudo_descriptor const d { idt, sizeof (idt) };
            asm volatile ("lidt %0" : : "m" (d));
        }
};
