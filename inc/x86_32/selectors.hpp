/*
 * Segment Selectors: x86_32
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

#define SEL_KERN_CODE   0x08
#define SEL_KERN_DATA   0x10
#define SEL_USER_DATA   0x1b    // RPL=3
#define SEL_USER_CODE   0x23    // RPL=3
#define SEL_TSS_RUN     0x28
#define SEL_MAX         0x30
