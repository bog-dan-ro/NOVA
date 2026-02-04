/*
 * Timer
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

#include "compiler.hpp"
#include "macros.hpp"
#include "types.hpp"

class Timer final
{
    private:
        static uint64_t freq;

    public:
        ALWAYS_INLINE
        static inline uint64_t time()
        {
            uint64_t val;
            asm volatile ("csrr %0, time" : "=r" (val));
            return val;
        }

        ALWAYS_INLINE
        static inline uint64_t frequency()
        {
            return freq;
        }

        ALWAYS_INLINE
        static inline void set_dln (uint64_t val)
        {
            // Set stimecmp (requires Sstc extension)
            // Fall back to SBI call if not available
            asm volatile ("csrw stimecmp, %0" : : "r" (val));
        }

        ALWAYS_INLINE
        static inline void stop()
        {
            // Disable timer interrupt
            asm volatile ("csrc sie, %0" : : "r" (BIT (5)));  // STIE
        }

        static void init();
};

