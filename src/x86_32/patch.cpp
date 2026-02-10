/*
 * Instruction Patching: x86_32
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

#include "memattr.hpp"
#include "patch.hpp"
#include "ptab_hpt.hpp"
#include "string.hpp"

void Patch::detect()
{
    // i486 has fixed 32-bit physical address width
    Memattr::obits = 32;
}

void Patch::init()
{
    extern Patch PATCH_S, PATCH_E;

    for (auto p { &PATCH_S }; p < &PATCH_E; p++) {

        if (applied & BIT (p->tag)) [[unlikely]] {

            auto const o { reinterpret_cast<uint8_t *>(p) + p->off_old };
            auto const n { reinterpret_cast<uint8_t *>(p) + p->off_new };

            memcpy (o, n, p->len_new);
            memset (o + p->len_new, NOP_OPC, p->len_old - p->len_new);
        }
    }
}
