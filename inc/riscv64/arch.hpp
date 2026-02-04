/*
 * Architecture Definitions: RISC-V
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

#define ARCH                "riscv64"
#define BFD_ARCH            "riscv"
#define BFD_FORMAT          "elf64-littleriscv"
#define ELF_MACHINE         Eh::Machine::RISCV

// Machine Status Register (mstatus)
#define MSTATUS_SD          BIT64 (63)              // State Dirty
#define MSTATUS_MBE         BIT64 (37)              // M-mode Big Endian
#define MSTATUS_SBE         BIT64 (36)              // S-mode Big Endian
#define MSTATUS_SXL         BIT64_RANGE (35, 34)    // S-mode XLEN
#define MSTATUS_UXL         BIT64_RANGE (33, 32)    // U-mode XLEN
#define MSTATUS_TSR         BIT64 (22)              // Trap SRET
#define MSTATUS_TW          BIT64 (21)              // Timeout Wait
#define MSTATUS_TVM         BIT64 (20)              // Trap Virtual Memory
#define MSTATUS_MXR         BIT64 (19)              // Make Executable Readable
#define MSTATUS_SUM         BIT64 (18)              // Supervisor User Memory
#define MSTATUS_MPRV        BIT64 (17)              // Modify Privilege
#define MSTATUS_XS          BIT64_RANGE (16, 15)    // Extension State
#define MSTATUS_FS          BIT64_RANGE (14, 13)    // FPU State
#define MSTATUS_MPP         BIT64_RANGE (12, 11)    // Machine Previous Privilege
#define MSTATUS_VS          BIT64_RANGE (10, 9)     // Vector State
#define MSTATUS_SPP         BIT64 (8)               // Supervisor Previous Privilege
#define MSTATUS_MPIE        BIT64 (7)               // Machine Previous Interrupt Enable
#define MSTATUS_UBE         BIT64 (6)               // U-mode Big Endian
#define MSTATUS_SPIE        BIT64 (5)               // Supervisor Previous Interrupt Enable
#define MSTATUS_MIE         BIT64 (3)               // Machine Interrupt Enable
#define MSTATUS_SIE         BIT64 (1)               // Supervisor Interrupt Enable

// Hypervisor Status Register (hstatus)
#define HSTATUS_VSXL        BIT64_RANGE (33, 32)    // VS-mode XLEN
#define HSTATUS_VTSR        BIT64 (22)              // Virtual Trap SRET
#define HSTATUS_VTW         BIT64 (21)              // Virtual Timeout Wait
#define HSTATUS_VTVM        BIT64 (20)              // Virtual Trap Virtual Memory
#define HSTATUS_VGEIN       BIT64_RANGE (17, 12)    // Virtual Guest External Interrupt Number
#define HSTATUS_HU          BIT64 (9)               // Hypervisor User Mode
#define HSTATUS_SPVP        BIT64 (8)               // Supervisor Previous Virtual Privilege
#define HSTATUS_SPV         BIT64 (7)               // Supervisor Previous Virtualization
#define HSTATUS_GVA         BIT64 (6)               // Guest Virtual Address
#define HSTATUS_VSBE        BIT64 (5)               // VS-mode Big Endian

// Supervisor Status Register (sstatus)
#define SSTATUS_SD          BIT64 (63)              // State Dirty
#define SSTATUS_UXL         BIT64_RANGE (33, 32)    // U-mode XLEN
#define SSTATUS_MXR         BIT64 (19)              // Make Executable Readable
#define SSTATUS_SUM         BIT64 (18)              // Supervisor User Memory
#define SSTATUS_XS          BIT64_RANGE (16, 15)    // Extension State
#define SSTATUS_FS          BIT64_RANGE (14, 13)    // FPU State
#define SSTATUS_VS          BIT64_RANGE (10, 9)     // Vector State
#define SSTATUS_SPP         BIT64 (8)               // Supervisor Previous Privilege
#define SSTATUS_UBE         BIT64 (6)               // U-mode Big Endian
#define SSTATUS_SPIE        BIT64 (5)               // Supervisor Previous Interrupt Enable
#define SSTATUS_SIE         BIT64 (1)               // Supervisor Interrupt Enable

// VS-mode Status Register (vsstatus)
#define VSSTATUS_SD         BIT64 (63)              // State Dirty
#define VSSTATUS_UXL        BIT64_RANGE (33, 32)    // U-mode XLEN
#define VSSTATUS_MXR        BIT64 (19)              // Make Executable Readable
#define VSSTATUS_SUM        BIT64 (18)              // Supervisor User Memory
#define VSSTATUS_XS         BIT64_RANGE (16, 15)    // Extension State
#define VSSTATUS_FS         BIT64_RANGE (14, 13)    // FPU State
#define VSSTATUS_VS         BIT64_RANGE (10, 9)     // Vector State
#define VSSTATUS_SPP        BIT64 (8)               // Supervisor Previous Privilege
#define VSSTATUS_UBE        BIT64 (6)               // U-mode Big Endian
#define VSSTATUS_SPIE       BIT64 (5)               // Supervisor Previous Interrupt Enable
#define VSSTATUS_SIE        BIT64 (1)               // Supervisor Interrupt Enable

// Supervisor Address Translation and Protection Register (satp)
#define SATP_MODE_BARE      VAL64_SHIFT (0, 60)     // No translation
#define SATP_MODE_SV39      VAL64_SHIFT (8, 60)     // 39-bit virtual addressing
#define SATP_MODE_SV48      VAL64_SHIFT (9, 60)     // 48-bit virtual addressing
#define SATP_MODE_SV57      VAL64_SHIFT (10, 60)    // 57-bit virtual addressing
#define SATP_ASID           BIT64_RANGE (59, 44)    // Address Space Identifier
#define SATP_PPN            BIT64_RANGE (43, 0)     // Physical Page Number

// Hypervisor Guest Address Translation and Protection Register (hgatp)
#define HGATP_MODE_BARE     VAL64_SHIFT (0, 60)     // No translation
#define HGATP_MODE_SV39X4   VAL64_SHIFT (8, 60)     // 39-bit guest physical addressing
#define HGATP_MODE_SV48X4   VAL64_SHIFT (9, 60)     // 48-bit guest physical addressing
#define HGATP_MODE_SV57X4   VAL64_SHIFT (10, 60)    // 57-bit guest physical addressing
#define HGATP_VMID          BIT64_RANGE (57, 44)    // Virtual Machine Identifier
#define HGATP_PPN           BIT64_RANGE (43, 0)     // Physical Page Number

// Supervisor Interrupt Enable/Pending (sie/sip)
#define SIE_SEIE            BIT64 (9)               // Supervisor External Interrupt Enable
#define SIE_STIE            BIT64 (5)               // Supervisor Timer Interrupt Enable
#define SIE_SSIE            BIT64 (1)               // Supervisor Software Interrupt Enable

// Hypervisor Interrupt Enable/Pending (hie/hip)
#define HIE_SGEIE           BIT64 (12)              // Supervisor Guest External Interrupt Enable
#define HIE_VSEIE           BIT64 (10)              // VS External Interrupt Enable
#define HIE_VSTIE           BIT64 (6)               // VS Timer Interrupt Enable
#define HIE_VSSIE           BIT64 (2)               // VS Software Interrupt Enable

// Hypervisor Virtual Interrupt Enable/Pending (hvie/hvip)
#define HVIE_VSEIE          BIT64 (10)              // VS External Interrupt Enable
#define HVIE_VSTIE          BIT64 (6)               // VS Timer Interrupt Enable
#define HVIE_VSSIE          BIT64 (2)               // VS Software Interrupt Enable

// Cause register exception codes
#define CAUSE_INT           BIT64 (63)              // Interrupt flag

// Interrupt causes
#define INT_SSI             1                       // Supervisor Software Interrupt
#define INT_VSI             2                       // Virtual Supervisor Software Interrupt
#define INT_MSI             3                       // Machine Software Interrupt
#define INT_STI             5                       // Supervisor Timer Interrupt
#define INT_VTI             6                       // Virtual Supervisor Timer Interrupt
#define INT_MTI             7                       // Machine Timer Interrupt
#define INT_SEI             9                       // Supervisor External Interrupt
#define INT_VEI             10                      // Virtual Supervisor External Interrupt
#define INT_MEI             11                      // Machine External Interrupt
#define INT_SGI             12                      // Supervisor Guest External Interrupt

// Exception causes
#define EXC_IAM             0                       // Instruction Address Misaligned
#define EXC_IAF             1                       // Instruction Access Fault
#define EXC_II              2                       // Illegal Instruction
#define EXC_BP              3                       // Breakpoint
#define EXC_LAM             4                       // Load Address Misaligned
#define EXC_LAF             5                       // Load Access Fault
#define EXC_SAM             6                       // Store/AMO Address Misaligned
#define EXC_SAF             7                       // Store/AMO Access Fault
#define EXC_ECU             8                       // Environment Call from U-mode
#define EXC_ECS             9                       // Environment Call from S-mode
#define EXC_ECVS            10                      // Environment Call from VS-mode
#define EXC_ECM             11                      // Environment Call from M-mode
#define EXC_IPF             12                      // Instruction Page Fault
#define EXC_LPF             13                      // Load Page Fault
#define EXC_SPF             15                      // Store/AMO Page Fault
#define EXC_IGPF            20                      // Instruction Guest-Page Fault
#define EXC_LGPF            21                      // Load Guest-Page Fault
#define EXC_VI              22                      // Virtual Instruction
#define EXC_SGPF            23                      // Store/AMO Guest-Page Fault

// Privilege levels
#define PRV_U               0                       // User
#define PRV_S               1                       // Supervisor
#define PRV_HS              2                       // Hypervisor-extended Supervisor
#define PRV_M               3                       // Machine

// Virtual privilege levels
#define PRV_VU              0                       // Virtual User
#define PRV_VS              1                       // Virtual Supervisor

