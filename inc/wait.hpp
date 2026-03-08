/*
 * Completion Wait
 *
 * Copyright (C) 2019-2026 Udo Steinberg, BlueRock Security, Inc.
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

#include "lowlevel.hpp"
#include "stc.hpp"
#include "timer.hpp"

/*
 * Spin-based polling helper with a millisecond wall-clock timeout.
 *
 * until(ms, func) repeatedly calls func() and pause() until func() returns
 * true or the deadline (converted to STC ticks) is exceeded. Returns true
 * if func() returned true within the deadline, false on timeout.
 */
class Wait final
{
    public:
        static bool until (uint32_t ms, auto const &func)
        {
            for (uint64_t const t { Stc::ms_to_ticks (ms) }, b { Timer::time() }; !func(); pause())
                if (Timer::time() - b > t) [[unlikely]]
                    return false;

            return true;
        }
};
