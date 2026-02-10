/*
 * Class Of Service (Stub): x86_32
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

#include "status.hpp"
#include "types.hpp"

class Cos final
{
    public:
        static void make_current (uint16_t) {}

        static bool valid_cos (cos_t) { return true; }

        static Status cfg (uint8_t, uint8_t, uint32_t) { return Status::BAD_FTR; }

        static Status cfg_qos (uint8_t) { return Status::BAD_FTR; }

        static Status cfg_l3_mask (uint16_t, uint32_t) { return Status::BAD_FTR; }

        static Status cfg_l2_mask (uint16_t, uint32_t) { return Status::BAD_FTR; }

        static Status cfg_mb_thrt (uint16_t, uint16_t) { return Status::BAD_FTR; }
};
