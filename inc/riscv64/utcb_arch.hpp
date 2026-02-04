/*
 * User Thread Control Block (UTCB): Architecture-Specific (RISC-V)
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

#include "mtd_arch.hpp"

struct Cpu_regs;
class Space_obj;

class Utcb_arch final
{
    private:
        // User-mode context (264 bytes)
        struct {
            uint64_t    gpr[31];        // x1-x31 (x0 is always zero) - 248 bytes
            uint64_t    sp;             // Stack Pointer - 8 bytes
            uint64_t    tp;             // Thread Pointer - 8 bytes
        } user;

        // Supervisor-mode CSRs - 48 bytes
        struct {
            uint64_t    sepc;           // Supervisor Exception PC
            uint64_t    sstatus;        // Supervisor Status
            uint64_t    scause;         // Supervisor Cause
            uint64_t    stval;          // Supervisor Trap Value
            uint64_t    satp;           // S-mode Address Translation and Protection
            uint64_t    sscratch;       // Supervisor Scratch
        } csr;

        // Space selector for guest assignment - 8 bytes
        struct {
            uint64_t    gst;
        } sel;

        bool assign_spaces (Cpu_regs &, Space_obj const *) const;

    public:
        void load (Mtd_arch const, Cpu_regs const &);
        bool save (Mtd_arch const, Cpu_regs &, Space_obj const *) const;
};

// 264 + 48 + 8 = 320 = 0x140
static_assert (__is_standard_layout (Utcb_arch) && sizeof (Utcb_arch) == 0x140);
