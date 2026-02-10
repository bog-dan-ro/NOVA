/*
 * Register File: x86_32
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
#include "hazard.hpp"
#include "selectors.hpp"
#include "space_hst.hpp"
#include "space_obj.hpp"
#include "space_pio.hpp"
#include "types.hpp"

struct Sys_regs
{
    uintptr_t   eax {};
    uintptr_t   ecx {};
    uintptr_t   edx {};
    uintptr_t   ebx {};
    uintptr_t   ebp {};
    uintptr_t   esi {};
    uintptr_t   edi {};
};

static_assert (__is_standard_layout (Sys_regs) && sizeof (Sys_regs) == __SIZEOF_POINTER__ * 7);

struct Exc_regs
{
    Sys_regs            sys;

    uintptr_t           err {};
    uintptr_t           vec {};
    uintptr_t           eip {};
    uintptr_t           cs  { SEL_USER_CODE };
    uintptr_t           efl { RFL_AC | RFL_IF | RFL_1 };
    uintptr_t           esp {};
    uintptr_t           ss  { SEL_USER_DATA };

    auto &ip() { return eip; }
    auto &sp() { return esp; }

    // CPL0/1/2 (supervisor) CPL3 (user)
    bool user() const { return (cs & 3) == 3; }

    auto ep() const { return vec; }

    void set_ep (uintptr_t val) { vec = val; }
};

static_assert (__is_standard_layout (Exc_regs) && sizeof (Exc_regs) == __SIZEOF_POINTER__ * 14);

class alignas (4) Cpu_regs final
{
    public:
        Exc_regs                exc;
        uintptr_t               cr2 {};
        Refptr<Space_obj> const obj;
        Refptr<Space_hst> const hst;
        Refptr<Space_pio>       pio     { nullptr };
        Hazard                  hazard  { 0 };

        Cpu_regs (Refptr<Space_obj> &o, Refptr<Space_hst> &h, Refptr<Space_pio> &p) : obj { std::move (o) }, hst { std::move (h) }, pio { std::move (p) } {}

        Space_obj *get_obj() const { return obj; }
        Space_hst *get_hst() const { return hst; }
        Space_pio *get_pio() const { return pio; }

        void fpu_ctrl (bool);
};
