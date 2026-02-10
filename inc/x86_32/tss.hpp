/*
 * Task State Segment (TSS): x86_32
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
#include "gdt.hpp"

class Tss final
{
    public:
        Unaligned_le<uint16_t>  prev;               // 0
        Unaligned_le<uint16_t>  res0;               // 2
        Unaligned_le<uint32_t>  esp0;               // 4
        Unaligned_le<uint16_t>  ss0;                // 8
        Unaligned_le<uint16_t>  res1;               // 10
        Unaligned_le<uint32_t>  esp1;               // 12
        Unaligned_le<uint16_t>  ss1;                // 16
        Unaligned_le<uint16_t>  res2;               // 18
        Unaligned_le<uint32_t>  esp2;               // 20
        Unaligned_le<uint16_t>  ss2;                // 24
        Unaligned_le<uint16_t>  res3;               // 26
        Unaligned_le<uint32_t>  cr3;                // 28
        Unaligned_le<uint32_t>  eip;                // 32
        Unaligned_le<uint32_t>  eflags;             // 36
        Unaligned_le<uint32_t>  eax;                // 40
        Unaligned_le<uint32_t>  ecx;                // 44
        Unaligned_le<uint32_t>  edx;                // 48
        Unaligned_le<uint32_t>  ebx;                // 52
        Unaligned_le<uint32_t>  esp;                // 56
        Unaligned_le<uint32_t>  ebp;                // 60
        Unaligned_le<uint32_t>  esi;                // 64
        Unaligned_le<uint32_t>  edi;                // 68
        Unaligned_le<uint16_t>  es;                 // 72
        Unaligned_le<uint16_t>  res4;               // 74
        Unaligned_le<uint16_t>  cs;                 // 76
        Unaligned_le<uint16_t>  res5;               // 78
        Unaligned_le<uint16_t>  ss;                 // 80
        Unaligned_le<uint16_t>  res6;               // 82
        Unaligned_le<uint16_t>  ds;                 // 84
        Unaligned_le<uint16_t>  res7;               // 86
        Unaligned_le<uint16_t>  fs;                 // 88
        Unaligned_le<uint16_t>  res8;               // 90
        Unaligned_le<uint16_t>  gs;                 // 92
        Unaligned_le<uint16_t>  res9;               // 94
        Unaligned_le<uint16_t>  ldt;                // 96
        Unaligned_le<uint16_t>  res10;              // 98
        Unaligned_le<uint16_t>  trap;               // 100
        Unaligned_le<uint16_t>  iobm;               // 102

        static Tss run asm ("tss_run")  CPULOCAL;

        static void build();

        static void load()
        {
            Gdt::unbusy_tss();
            asm volatile ("ltr %w0" : : "rm" (SEL_TSS_RUN));
        }
};

static_assert (__is_standard_layout (Tss) && alignof (Tss) == 1 && sizeof (Tss) == 104);
