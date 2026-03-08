/*
 * Board-Specific Configuration: QEMU Virtual Board
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

#define RAM_BASE    0x80000000  // 0x80000000 - ...
#define RAM_SIZE    0x10000000  // 256MB

#ifndef __ASSEMBLER__

struct Board
{
    // RISC-V QEMU virt machine memory map
    static constexpr unsigned long PLIC_BASE  = 0x0c000000;     // Platform-Level Interrupt Controller

    static constexpr struct {
        unsigned long mmio;
    } plic { PLIC_BASE };
};

#endif
