/*
 * 8259A Programmable Interrupt Controller (PIC): x86_32
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

#include "io.hpp"
#include "macros.hpp"
#include "types.hpp"
#include "vectors.hpp"

class Pic final
{
    public:
        static void init()
        {
            // Master PIC
            Io::out<uint8_t>(0x20, BIT (4) | BIT (0));  // ICW1: ICW4 needed
            Io::out<uint8_t>(0x21, VEC_GSI);            // ICW2: Interrupt vector base
            Io::out<uint8_t>(0x21, BIT (2));            // ICW3: Slave on pin 2
            Io::out<uint8_t>(0x21, BIT (0));            // ICW4: 8086 mode
            Io::out<uint8_t>(0x21, BIT_RANGE (7, 0));   // OCW1: Mask everything

            // Slave PIC
            Io::out<uint8_t>(0xa0, BIT (4) | BIT (0));  // ICW1: ICW4 needed
            Io::out<uint8_t>(0xa1, VEC_GSI + 8);        // ICW2: Interrupt vector base
            Io::out<uint8_t>(0xa1, 2);                  // ICW3: Cascade identity
            Io::out<uint8_t>(0xa1, BIT (0));            // ICW4: 8086 mode
            Io::out<uint8_t>(0xa1, BIT_RANGE (7, 0));   // OCW1: Mask everything
        }

        static void eoi (unsigned vec)
        {
            if (vec >= VEC_GSI + 8)
                Io::out<uint8_t>(0xa0, 0x20);           // Non-specific EOI to slave
            Io::out<uint8_t>(0x20, 0x20);               // Non-specific EOI to master
        }

        static void unmask (unsigned irq)
        {
            if (irq < 8)
                Io::out<uint8_t>(0x21, static_cast<uint8_t>(Io::in<uint8_t>(0x21) & ~BIT (irq)));
            else
                Io::out<uint8_t>(0xa1, static_cast<uint8_t>(Io::in<uint8_t>(0xa1) & ~BIT (irq - 8)));
        }

        static void mask (unsigned irq)
        {
            if (irq < 8)
                Io::out<uint8_t>(0x21, static_cast<uint8_t>(Io::in<uint8_t>(0x21) | BIT (irq)));
            else
                Io::out<uint8_t>(0xa1, static_cast<uint8_t>(Io::in<uint8_t>(0xa1) | BIT (irq - 8)));
        }
};
