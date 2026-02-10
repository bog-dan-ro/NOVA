/*
 * Timer: Architecture-Specific (x86_32)
 *
 * Uses the 8254 PIT for timing on i486 (no LAPIC/TSC)
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
#include "stc.hpp"
#include "timeout.hpp"

class Timer final : private Stc
{
    private:
        // PIT oscillator frequency: 1193182 Hz
        static constexpr uint32_t pit_freq { 1193182 };

        static inline uint64_t ticks { 0 };

    public:
        static auto time()
        {
            return ticks;
        }

        static void set_dln (uint64_t)
        {
            // PIT is one-shot or periodic; simplified for now
        }

        static void stop() {}

        static void set_time (uint64_t t)
        {
            ticks = t;
        }

        static void tick()
        {
            ticks++;
            Timeout::check();
        }

        static void init()
        {
            // Channel 0, Rate Generator, lo/hi byte access
            Io::out<uint8_t>(0x43, 0x34);

            // ~1000 Hz (divider = 1193)
            constexpr uint16_t div { static_cast<uint16_t>(pit_freq / 1000) };
            Io::out<uint8_t>(0x40, static_cast<uint8_t>(div & 0xff));
            Io::out<uint8_t>(0x40, static_cast<uint8_t>(div >> 8));
        }
};
