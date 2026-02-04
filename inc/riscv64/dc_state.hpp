/*
 * Device Context (DC) State
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

class Smmu;

/*
 * RISC-V minimal Device Context State
 *
 * Full IOMMU support (e.g., RISC-V IOMMU) would be added later.
 */
class Dc_state
{
    public:
        uint32_t const  did     { 0 };
        uint32_t const  sid     { 0 };
        Smmu * const    smmu    { nullptr };    // No IOMMU support

    protected:
        explicit Dc_state (uint64_t, uint64_t, uint64_t) {}

        ~Dc_state() = default;
};
