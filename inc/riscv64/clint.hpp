/*
 * CLINT (Core Local Interruptor)
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

#include "board.hpp"
#include "types.hpp"

class Clint
{
    private:
        // CLINT register offsets
        static constexpr unsigned MSIP_BASE     { 0x0000 };     // Machine Software Interrupt Pending
        static constexpr unsigned MTIMECMP_BASE { 0x4000 };     // Machine Timer Compare
        static constexpr unsigned MTIME         { 0xbff8 };     // Machine Timer

        static inline uintptr_t clint_base { 0 };

        static inline uint64_t read64 (unsigned reg)
        {
            return *reinterpret_cast<uint64_t volatile *>(clint_base + reg);
        }

        static inline void write64 (unsigned reg, uint64_t val)
        {
            *reinterpret_cast<uint64_t volatile *>(clint_base + reg) = val;
        }

        static inline uint32_t read32 (unsigned reg)
        {
            return *reinterpret_cast<uint32_t volatile *>(clint_base + reg);
        }

        static inline void write32 (unsigned reg, uint32_t val)
        {
            *reinterpret_cast<uint32_t volatile *>(clint_base + reg) = val;
        }

    public:
        static void init();

        // Read current time
        static uint64_t time()
        {
            return read64 (MTIME);
        }

        // Set timer compare for a hart
        static void set_timecmp (unsigned hart, uint64_t val)
        {
            write64 (MTIMECMP_BASE + hart * 8, val);
        }

        // Send IPI to a hart
        static void send_ipi (unsigned hart)
        {
            write32 (MSIP_BASE + hart * 4, 1);
        }

        // Clear IPI for a hart
        static void clear_ipi (unsigned hart)
        {
            write32 (MSIP_BASE + hart * 4, 0);
        }
};

