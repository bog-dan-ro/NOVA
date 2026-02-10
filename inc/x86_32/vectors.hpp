/*
 * Interrupt Vectors: x86_32
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

#include "config.hpp"

// No LAPIC LVT vectors; PIC-only system
#define NUM_FLT         1
#define NUM_IPI         2
#define NUM_LVT         0
#define NUM_GSI         (NUM_VEC - NUM_EXC - NUM_FLT)

#define VEC_GSI         NUM_EXC
#define VEC_FLT         (VEC_GSI + NUM_GSI)
