/*
 * System Memory Management Unit - RISC-V Stub
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

#include "sdid.hpp"
#include "status.hpp"
#include "types.hpp"

class Dc;
class Space_dma;

/*
 * RISC-V IOMMU stub
 *
 * Full IOMMU support would require the RISC-V IOMMU specification.
 */
class Smmu
{
    public:
        static uint8_t avail_smg() { return 0; }
        static uint8_t avail_ctx() { return 0; }

        [[nodiscard]] static bool initialize() { return true; }

        static Smmu *lookup (uint64_t) { return nullptr; }

        static bool using_iid (unsigned) { return false; }

        static void interrupt (unsigned) {}

        static void tlb_invalidate_all (Sdid) {}

        Status assign_dev (Dc const *, Space_dma *) { return Status::BAD_FTR; }
};
