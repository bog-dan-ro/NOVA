/*
 * DMA Space (Stub): x86_32
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

#include "memattr.hpp"
#include "paging.hpp"
#include "space_mem.hpp"
#include "status.hpp"

class Pd;

class Space_dma final : public Space_mem<Space_dma>
{
    private:
        Space_dma (Refptr<Pd> &ref_pd) : Space_mem { Kobject::Subtype::DMA, ref_pd } {}

        void collect() override final {}

    public:
        static constexpr uint64_t selectors { 0 };

        static auto mco() { return static_cast<uint8_t>(0); }

        [[nodiscard]] static Space_dma *create (Status &s, Pd *)
        {
            s = Status::BAD_FTR;
            return nullptr;
        }

        void destroy() override final {}

        auto update (uint64_t, uint64_t, unsigned, Paging::Permissions, Memattr) { return Status::BAD_FTR; }

        void sync() {}
};
