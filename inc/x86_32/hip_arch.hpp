/*
 * Hypervisor Information Page (HIP): Architecture-Specific Part (x86_32)
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
        uint8_t     num_vec;
        uint16_t    num_pin;

    public:
        enum class Feature : uint64_t
        {
            SMMU    = 0,    // No IOMMU on i486
        };

        void build();
};

static_assert (__is_standard_layout (Hip_arch));
