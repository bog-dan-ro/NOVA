/*
 * Architecture Definitions: x86_32
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

#define ARCH                "x86_32"
#define BFD_ARCH            "i386"
#define BFD_FORMAT          "elf32-i386"
#define ELF_MACHINE         Eh::Machine::X86_32

#define EXC_DE              0                       // Divide Error
#define EXC_DB              1                       // Debug Exception
#define EXC_NMI             2                       // Non-Maskable Interrupt
#define EXC_BP              3                       // Breakpoint
#define EXC_OF              4                       // Overflow
#define EXC_BR              5                       // Bound Range Exceeded
#define EXC_UD              6                       // Undefined Opcode
#define EXC_NM              7                       // No Math Coprocessor
#define EXC_DF              8                       // Double Fault
#define EXC_TS              10                      // Invalid TSS
#define EXC_NP              11                      // Segment Not Present
#define EXC_SS              12                      // Stack-Segment Fault
#define EXC_GP              13                      // General Protection Fault
#define EXC_PF              14                      // Page Fault
#define EXC_MF              16                      // Math Fault
#define EXC_AC              17                      // Alignment Check

#define IDT_MASK            BIT_RANGE (1, 0)
#define IDT_IST1            BIT  (1)                // Stack Switching (unused on i486, kept for compatibility)
#define IDT_USER            BIT  (0)                // User Accessible

#define RFL_ID              BIT (21)                // Identification Flag
#define RFL_AC              BIT (18)                // Alignment Check Flag
#define RFL_VM              BIT (17)                // Virtual-8086 Mode Flag
#define RFL_RF              BIT (16)                // Resume Flag
#define RFL_NT              BIT (14)                // Nested Task Flag
#define RFL_IOPL            BIT_RANGE (13, 12)      // I/O Privilege Level
#define RFL_OF              BIT (11)                // Overflow Flag
#define RFL_DF              BIT (10)                // Direction Flag
#define RFL_IF              BIT  (9)                // Interrupt Enable Flag
#define RFL_TF              BIT  (8)                // Trap Flag
#define RFL_SF              BIT  (7)                // Sign Flag
#define RFL_ZF              BIT  (6)                // Zero Flag
#define RFL_AF              BIT  (4)                // Auxiliary Carry Flag
#define RFL_PF              BIT  (2)                // Parity Flag
#define RFL_1               BIT  (1)                // Must be 1
#define RFL_CF              BIT  (0)                // Carry Flag

#define CR0_PG              BIT (31)                // Paging
#define CR0_CD              BIT (30)                // Cache Disable
#define CR0_NW              BIT (29)                // Not Write-Through
#define CR0_AM              BIT (18)                // Alignment Mask
#define CR0_WP              BIT (16)                // Write Protect
#define CR0_NE              BIT  (5)                // Numeric Error
#define CR0_ET              BIT  (4)                // Extension Type
#define CR0_TS              BIT  (3)                // Task Switched
#define CR0_EM              BIT  (2)                // Emulation
#define CR0_MP              BIT  (1)                // Monitor Coprocessor
#define CR0_PE              BIT  (0)                // Protection Enable

#define CR4_PSE             BIT  (4)                // Page Size Extensions
#define CR4_DE              BIT  (3)                // Debugging Extensions
#define CR4_TSD             BIT  (2)                // Time Stamp Disable
#define CR4_PVI             BIT  (1)                // Protected-Mode Virtual Interrupts
#define CR4_VME             BIT  (0)                // Virtual-8086 Mode Extensions
