/*
 * Interrupt Identifier
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

#include "types.hpp"

class Intid
{
    public:
        // RISC-V interrupt types
        // Software interrupts (IPI)
        static constexpr unsigned NUM_IPI   { 1 };      // Supervisor software interrupt

        // Local interrupts (timer, etc.)
        static constexpr unsigned NUM_LOC   { 2 };      // Timer + software

        // External interrupts via PLIC
        static constexpr unsigned NUM_EXT   { 1024 };   // Maximum PLIC sources

        // Total interrupts
        static constexpr unsigned NUM_INT   { NUM_IPI + NUM_LOC + NUM_EXT };

        enum class Type : unsigned
        {
            IPI,        // Software interrupt (for IPI)
            TIMER,      // Timer interrupt
            EXT,        // External interrupt (via PLIC)
            UNKNOWN,
        };

        // RISC-V interrupt causes
        static constexpr unsigned CAUSE_SSI { 1 };      // Supervisor Software Interrupt
        static constexpr unsigned CAUSE_STI { 5 };      // Supervisor Timer Interrupt
        static constexpr unsigned CAUSE_SEI { 9 };      // Supervisor External Interrupt

        static constexpr auto type (unsigned cause)
        {
            switch (cause) {
                case CAUSE_SSI: return Type::IPI;
                case CAUSE_STI: return Type::TIMER;
                case CAUSE_SEI: return Type::EXT;
                default:        return Type::UNKNOWN;
            }
        }
};

