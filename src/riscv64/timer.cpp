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

#include "timer.hpp"
#include "arch.hpp"
#include "console.hpp"

uint64_t Timer::freq;

void Timer::init()
{
    // RISC-V timer frequency is typically provided by device tree
    // or can be queried via SBI (timebase-frequency)
    // Default to 10MHz for QEMU virt machine
    freq = 10'000'000;

    // Enable timer interrupt
    asm volatile ("csrs sie, %0" : : "r" (BIT (5)));  // STIE

    // Set initial timer compare far in the future
    set_dln (time() + freq);

    Console::print ("TIMER: RISC-V timer @ %lu Hz\n", freq);
}

