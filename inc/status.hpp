/*
 * Hypercall Status Codes
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
 * Copyright (C) 2019-2026 Udo Steinberg, BlueRock Security, Inc.
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

/*
 * Hypercall return status codes.
 *
 * Returned in the first register/UTCB word of the syscall ABI. Values map
 * directly to the binary encoding seen by user-space.
 */
enum class Status : unsigned
{
    SUCCESS,    // Operation completed successfully
    TIMEOUT,    // Blocking operation timed out
    ABORTED,    // Operation aborted (e.g. semaphore destroyed while EC was waiting)
    OVRFLOW,    // Counter overflow (e.g. SM up() when counter is at maximum)
    BAD_HYP,    // Unknown hypercall number
    BAD_CAP,    // Invalid capability or insufficient permissions
    BAD_PAR,    // Invalid parameter value
    BAD_FTR,    // Unsupported feature
    BAD_CPU,    // Invalid CPU identifier
    BAD_DEV,    // Invalid device identifier
    MEM_OBJ,    // Out of kernel object memory
    MEM_CAP,    // Out of capability space memory
};
