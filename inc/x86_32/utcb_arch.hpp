/*
 * User Thread Control Block (UTCB): Architecture-Specific (x86_32)
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
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

#include "mtd_arch.hpp"

class Cpu_regs;
struct Exc_regs;
class Space_obj;

class Utcb_arch final
{
    private:
        uint32_t        eax, ecx, edx, ebx, esp, ebp, esi, edi;
        uint32_t        efl, eip;
        uint32_t        cr2;

    public:
        void load_exc (Mtd_arch const, Cpu_regs const &);
        bool save_exc (Mtd_arch const, Cpu_regs &) const;
};

static_assert (__is_standard_layout (Utcb_arch));
