/*
 * PLIC (Platform-Level Interrupt Controller)
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

#include "board.hpp"
#include "memory.hpp"
#include "types.hpp"

class Plic
{
    private:
        static constexpr unsigned MAX_SOURCES   { 1024 };
        static constexpr unsigned MAX_CONTEXTS  { 15872 };

        // PLIC register offsets
        static constexpr unsigned PRIORITY_BASE { 0x000000 };   // Priority registers (4 bytes per source)
        static constexpr unsigned PENDING_BASE  { 0x001000 };   // Pending bits (1 bit per source)
        static constexpr unsigned ENABLE_BASE   { 0x002000 };   // Enable bits per context
        static constexpr unsigned CONTEXT_BASE  { 0x200000 };   // Context registers (threshold + claim/complete)

        static constexpr unsigned CONTEXT_SIZE  { 0x1000 };     // Size per context
        static constexpr unsigned ENABLE_SIZE   { 0x80 };       // Enable size per context

        static inline uintptr_t plic_base { 0 };

        static inline uint32_t read (unsigned reg)
        {
            return *reinterpret_cast<uint32_t volatile *>(plic_base + reg);
        }

        static inline void write (unsigned reg, uint32_t val)
        {
            *reinterpret_cast<uint32_t volatile *>(plic_base + reg) = val;
        }

    public:
        static void init (cpu_t);
        static void conf (unsigned, bool);
        static void send_sgi (unsigned, cpu_t);

        // Set priority for an interrupt source
        static void set_priority (unsigned source, unsigned priority)
        {
            write (PRIORITY_BASE + source * 4, priority);
        }

        // Enable interrupt for a context
        static void enable (unsigned context, unsigned source)
        {
            unsigned reg { ENABLE_BASE + context * ENABLE_SIZE + (source / 32) * 4 };
            write (reg, read (reg) | (1U << (source % 32)));
        }

        // Disable interrupt for a context
        static void disable (unsigned context, unsigned source)
        {
            unsigned reg { ENABLE_BASE + context * ENABLE_SIZE + (source / 32) * 4 };
            write (reg, read (reg) & ~(1U << (source % 32)));
        }

        // Set threshold for a context
        static void set_threshold (unsigned context, unsigned threshold)
        {
            write (CONTEXT_BASE + context * CONTEXT_SIZE, threshold);
        }

        // Claim an interrupt (returns source ID, 0 if none)
        static unsigned claim (unsigned context)
        {
            return read (CONTEXT_BASE + context * CONTEXT_SIZE + 4);
        }

        // Complete an interrupt
        static void complete (unsigned context, unsigned source)
        {
            write (CONTEXT_BASE + context * CONTEXT_SIZE + 4, source);
        }
};

