/*
 * Hypervisor Information Page (HIP): Architecture-Specific Part (RISC-V)
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

#include "macros.hpp"
#include "types.hpp"

class Hip_arch final
{
    private:
        uint32_t    num_plic_ctx;       // Number of PLIC contexts
        uint32_t    num_plic_irq;       // Number of PLIC interrupts
        uint64_t    reserved;

    public:
        enum class Feature : uint64_t
        {
            SMMU    = BIT (0),          // IOMMU (for compatibility with common code)
            PLIC    = BIT (1),          // Platform-Level Interrupt Controller
            CLINT   = BIT (2),          // Core-Local Interruptor
            IOMMU   = BIT (3),          // RISC-V IOMMU
        };

        void build();
};

static_assert (__is_standard_layout (Hip_arch) && sizeof (Hip_arch) == 16);
