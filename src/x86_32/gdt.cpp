/*
 * Global Descriptor Table (GDT): x86_32
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

#include "gdt.hpp"
#include "memory.hpp"
#include "tss.hpp"

ALIGNED(8) Gdt Gdt::gdt;

void Gdt::build()
{
    gdt.kern_code = Descriptor_gdt_seg { Descriptor_gdt_seg::Type::CODE_XRA, 0 };
    gdt.kern_data = Descriptor_gdt_seg { Descriptor_gdt_seg::Type::DATA_RWA, 0 };
    gdt.user_data = Descriptor_gdt_seg { Descriptor_gdt_seg::Type::DATA_RWA, 3 };
    gdt.user_code = Descriptor_gdt_seg { Descriptor_gdt_seg::Type::CODE_XRA, 3 };

    gdt.tss_run = Descriptor_gdt_sys { Descriptor_gdt_sys::Type::SYS_TSS, reinterpret_cast<uintptr_t>(&Tss::run), sizeof (Tss) - 1 };
}
