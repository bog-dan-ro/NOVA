/*
 * External Symbols
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

#include "types.hpp"

extern char GIT_VER, NOVA_HPAS, NOVA_HPAE, PTAB_HPAS, PT2H_HPAS, PT1H_HPAS, KMEM_HVAS, DSTK_TOP, STACK;
extern char __bss_start, __bss_end;
extern void (*CTORS_S[])(), (*CTORS_E[])(), (*CTORS_C[])(), (*CTORS_L[])();

// Boot parameters from start.S
extern uint64_t boot_hartid;
extern uint64_t boot_dtb;

