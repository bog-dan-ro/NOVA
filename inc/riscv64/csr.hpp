/*
 * CSR (Control and Status Register) Access
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

#include "compiler.hpp"
#include "types.hpp"

class Csr
{
    public:
        // Supervisor CSRs (0x100-0x1FF)
        static constexpr unsigned SSTATUS       = 0x100;
        static constexpr unsigned SIE           = 0x104;
        static constexpr unsigned STVEC         = 0x105;
        static constexpr unsigned SCOUNTEREN    = 0x106;
        static constexpr unsigned SENVCFG       = 0x10A;
        static constexpr unsigned SSCRATCH      = 0x140;
        static constexpr unsigned SEPC          = 0x141;
        static constexpr unsigned SCAUSE        = 0x142;
        static constexpr unsigned STVAL         = 0x143;
        static constexpr unsigned SIP           = 0x144;
        static constexpr unsigned STIMECMP      = 0x14D;    // Sstc extension
        static constexpr unsigned SATP          = 0x180;

        // Hypervisor CSRs (0x600-0x6FF, 0xA00-0xAFF)
        static constexpr unsigned HSTATUS       = 0x600;
        static constexpr unsigned HEDELEG       = 0x602;
        static constexpr unsigned HIDELEG       = 0x603;
        static constexpr unsigned HIE           = 0x604;
        static constexpr unsigned HCOUNTEREN    = 0x606;
        static constexpr unsigned HGEIE         = 0x607;
        static constexpr unsigned HTVAL         = 0x643;
        static constexpr unsigned HIP           = 0x644;
        static constexpr unsigned HVIP          = 0x645;
        static constexpr unsigned HTINST        = 0x64A;
        static constexpr unsigned HGEIP         = 0xE12;
        static constexpr unsigned HENVCFG       = 0x60A;
        static constexpr unsigned HGATP         = 0x680;
        static constexpr unsigned HTIMEDELTA    = 0x605;

        // Virtual Supervisor CSRs (0x200-0x2FF)
        static constexpr unsigned VSSTATUS      = 0x200;
        static constexpr unsigned VSIE          = 0x204;
        static constexpr unsigned VSTVEC        = 0x205;
        static constexpr unsigned VSSCRATCH     = 0x240;
        static constexpr unsigned VSEPC         = 0x241;
        static constexpr unsigned VSCAUSE       = 0x242;
        static constexpr unsigned VSTVAL        = 0x243;
        static constexpr unsigned VSIP          = 0x244;
        static constexpr unsigned VSATP         = 0x280;

        // Read CSR
        template <unsigned N>
        ALWAYS_INLINE
        static inline uint64_t read()
        {
            uint64_t val;
            asm volatile ("csrr %0, %1" : "=r" (val) : "i" (N));
            return val;
        }

        // Write CSR
        template <unsigned N>
        ALWAYS_INLINE
        static inline void write (uint64_t val)
        {
            asm volatile ("csrw %0, %1" : : "i" (N), "r" (val));
        }

        // Set bits in CSR
        template <unsigned N>
        ALWAYS_INLINE
        static inline void set (uint64_t val)
        {
            asm volatile ("csrs %0, %1" : : "i" (N), "r" (val));
        }

        // Clear bits in CSR
        template <unsigned N>
        ALWAYS_INLINE
        static inline void clr (uint64_t val)
        {
            asm volatile ("csrc %0, %1" : : "i" (N), "r" (val));
        }
};

