/*
 * Descriptor: x86_32
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
#include "endian.hpp"
#include "macros.hpp"
#include "std.hpp"

/*
 * Descriptor Base Class
 */
template<unsigned N> struct Descriptor
{
    uint32_t val[N];

    enum class Type : unsigned
    {
        // System Segments
        SYS_LDT                 = 0x2,
        SYS_TSS                 = 0x9,
        SYS_CALL_GATE           = 0xc,
        SYS_INTR_GATE           = 0xe,
        SYS_TRAP_GATE           = 0xf,

        // Data Segments
        DATA_R                  = 0x10,
        DATA_RA                 = 0x11,
        DATA_RW                 = 0x12,
        DATA_RWA                = 0x13,
        DATA_DOWN_R             = 0x14,
        DATA_DOWN_RA            = 0x15,
        DATA_DOWN_RW            = 0x16,
        DATA_DOWN_RWA           = 0x17,

        // Code Segments
        CODE_X                  = 0x18,
        CODE_XA                 = 0x19,
        CODE_XR                 = 0x1a,
        CODE_XRA                = 0x1b,
        CODE_CONF_X             = 0x1c,
        CODE_CONF_XA            = 0x1d,
        CODE_CONF_XR            = 0x1e,
        CODE_CONF_XRA           = 0x1f,
    };
};

/*
 * Descriptor: Page-Granular 32-Bit Code/Data Segment
 *
 * 32-bit segment descriptor:
 *   val[0] = base[15:0] << 16 | limit[15:0]
 *   val[1] = base[31:24] << 24 | flags << 20 | limit[19:16] << 16 | access << 8 | base[23:16]
 *
 * For flat segments: base=0, limit=0xfffff, G=1, D/B=1
 */
struct Descriptor_gdt_seg final : public Descriptor<2>
{
    explicit constexpr Descriptor_gdt_seg() : Descriptor { 0, 0 } {}

    /*
     * Constructor for flat 32-bit segment
     *
     * @param t Descriptor Type
     * @param d Descriptor Privilege Level (0...3)
     */
    explicit constexpr Descriptor_gdt_seg (Type t, unsigned d) : Descriptor {
        0x0000ffff,     // base=0, limit[15:0]=0xffff
        static_cast<uint32_t>(BIT (23) | BIT (22) | BIT (15) | (0xf << 16) | d << 13 | std::to_underlying (t) << 8)
        // G=1 (bit 23), D/B=1 (bit 22), P=1 (bit 15), limit[19:16]=0xf
    } {}
};

static_assert (__is_standard_layout (Descriptor_gdt_seg) && sizeof (Descriptor_gdt_seg) == 8);

/*
 * Descriptor: Byte-Granular 32-Bit System Segment (TSS, LDT)
 */
struct Descriptor_gdt_sys final : public Descriptor<2>
{
    explicit constexpr Descriptor_gdt_sys() : Descriptor { 0, 0 } {}

    /*
     * Constructor
     *
     * @param t Descriptor Type
     * @param b Segment Base Address
     * @param l Segment Limit
     */
    explicit constexpr Descriptor_gdt_sys (Type t, uint32_t b, uint32_t l) : Descriptor {
        static_cast<uint32_t>(b << 16 | (l & BIT_RANGE (15, 0))),
        static_cast<uint32_t>((b & BIT_RANGE (31, 24)) | (l & BIT_RANGE (19, 16)) | BIT (15) | std::to_underlying (t) << 8 | (b >> 16 & BIT_RANGE (7, 0)))
    } {}
};

static_assert (__is_standard_layout (Descriptor_gdt_sys) && sizeof (Descriptor_gdt_sys) == 8);

/*
 * Descriptor: 32-Bit IDT Gate (Interrupt/Trap)
 */
struct Descriptor_idt final : public Descriptor<2>
{
    explicit constexpr Descriptor_idt() : Descriptor { 0, 0 } {}

    /*
     * Constructor
     *
     * @param d Descriptor Privilege Level (0...3)
     * @param s Segment Selector (Destination Code Segment)
     * @param e Entry Instruction Pointer (Interrupt Handler)
     */
    explicit constexpr Descriptor_idt (unsigned d, unsigned, uint16_t s, uint32_t e) : Descriptor {
        static_cast<uint32_t>(s << 16 | (e & BIT_RANGE (15, 0))),
        static_cast<uint32_t>((e & BIT_RANGE (31, 16)) | BIT (15) | d << 13 | std::to_underlying (Type::SYS_INTR_GATE) << 8)
    } {}
};

static_assert (__is_standard_layout (Descriptor_idt) && sizeof (Descriptor_idt) == 8);

/*
 * Pseudo Descriptor for LGDT/LIDT
 */
class Pseudo_descriptor final
{
    private:
        Unaligned_le<uint16_t>      limit;
        Unaligned_le<uintptr_t>     base;

    public:
        explicit Pseudo_descriptor (void *b, size_t l) : limit { static_cast<uint16_t>(l - 1) }, base { reinterpret_cast<uintptr_t>(b) } {}
};

static_assert (__is_standard_layout (Pseudo_descriptor) && alignof (Pseudo_descriptor) == 1 && sizeof (Pseudo_descriptor) == 6);
