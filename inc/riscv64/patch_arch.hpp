/*
 * Patch (Runtime Code Patching) - RISC-V
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

// RISC-V NOP instruction: ADDI x0, x0, 0 = 0x00000013
// For compressed: C.NOP = 0x0001 (2 bytes)
// Using uncompressed NOP for simplicity
#define NOP_LEN         4
#define NOP_OPC         0x00000013

class Patch_arch
{
    public:
        static void init() {}
};

