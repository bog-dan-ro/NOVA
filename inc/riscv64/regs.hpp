/*
 * Register File
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

#include "arch.hpp"
#include "compiler.hpp"
#include "hazard.hpp"
#include "space_hst.hpp"
#include "space_obj.hpp"
#include "types.hpp"

// Forward declarations
class Space_pio;

struct Sys_regs
{
    // x1-x31 (x0 is always zero)
    uintptr_t   gpr[31]     { 0 };
};

static_assert (__is_standard_layout (Sys_regs) && sizeof (Sys_regs) == __SIZEOF_POINTER__ * 31);

struct Exc_regs
{
    Sys_regs    sys;

    struct {
        uint64_t  sepc      { 0 };      // Supervisor Exception PC
        uint64_t  sstatus   { 0 };      // Supervisor Status
        uint64_t  scause    { 0 };      // Supervisor Cause
        uint64_t  stval     { 0 };      // Supervisor Trap Value
    } csr;

    inline auto &ip()       { return csr.sepc; }
    inline auto &sp()       { return sys.gpr[1]; }  // x2 (sp) is at index 1

    inline unsigned mode() const { return (csr.sstatus & SSTATUS_SPP) ? PRV_S : PRV_U; }

    inline auto cause() const { return csr.scause & ~CAUSE_INT; }
    inline auto is_interrupt() const { return csr.scause & CAUSE_INT; }

    // Event portal selector - based on exception cause
    inline auto ep() const { return static_cast<unsigned>(cause()); }
    inline void set_ep (uint64_t val) { csr.scause = val; }

    inline void set_cause (uint64_t val) { csr.scause = val; }
};

static_assert (__is_standard_layout (Exc_regs) && sizeof (Exc_regs) == __SIZEOF_POINTER__ * 35);

struct alignas (16) Cpu_regs final
{
    Exc_regs                exc;
    Refptr<Space_obj> const obj;
    Refptr<Space_hst> const hst;
    Hazard                  hazard  { 0 };

    Cpu_regs (Refptr<Space_obj> &o, Refptr<Space_hst> &h, Refptr<Space_pio> &) : obj { std::move (o) }, hst { std::move (h) } {}

    Space_obj *get_obj() const { return obj; }
    Space_hst *get_hst() const { return hst; }
};
