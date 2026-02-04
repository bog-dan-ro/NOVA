/*
 * Interrupt Handling
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

#include "interrupt.hpp"
#include "arch.hpp"
#include "console.hpp"
#include "counter.hpp"
#include "intid.hpp"
#include "timer.hpp"

void Interrupt::init()
{
    // Enable supervisor external interrupt
    asm volatile ("csrs sie, %0" : : "r" (SIE_SEIE | SIE_STIE | SIE_SSIE));

    Console::print ("INTR: RISC-V interrupt controller initialized\n");
}

void Interrupt::handle_irq()
{
    uint64_t cause;
    asm volatile ("csrr %0, scause" : "=r" (cause));

    // Extract interrupt cause (lower bits)
    unsigned irq { static_cast<unsigned>(cause & ~CAUSE_INT) };

    switch (irq) {
        case INT_SSI:   // Supervisor Software Interrupt (IPI)
            handle_ipi();
            break;
        case INT_STI:   // Supervisor Timer Interrupt
            handle_timer();
            break;
        case INT_SEI:   // Supervisor External Interrupt
            // Would query PLIC for interrupt source
            break;
        default:
            Console::print ("INTR: Unknown interrupt %u\n", irq);
            break;
    }
}

void Interrupt::handle_ipi()
{
    Counter::req[0].inc();

    // Clear software interrupt pending
    asm volatile ("csrc sip, %0" : : "r" (SIE_SSIE));
}

void Interrupt::handle_timer()
{
    Counter::loc[0].inc();

    // Disable timer interrupt and reschedule
    Timer::stop();

    // Re-arm timer (would normally be done by scheduler)
    Timer::set_dln (Timer::time() + Timer::frequency());
    asm volatile ("csrs sie, %0" : : "r" (SIE_STIE));
}

void Interrupt::send_ipi (unsigned hart, unsigned)
{
    // Use SBI to send IPI
    // sbi_send_ipi (hart_mask, hart_mask_base)
    register unsigned long a7 asm ("a7") = 0x735049;    // SBI_EXT_IPI
    register unsigned long a6 asm ("a6") = 0x0;         // sbi_send_ipi
    register unsigned long a0 asm ("a0") = 1UL << hart; // Hart mask
    register unsigned long a1 asm ("a1") = 0;           // Hart mask base

    asm volatile ("ecall" : "+r" (a0) : "r" (a1), "r" (a6), "r" (a7));
}

void Interrupt::conf (unsigned, bool)
{
    // Configure interrupt via PLIC
}

void Interrupt::deactivate (unsigned)
{
    // Deactivate interrupt via PLIC
}

void Interrupt::send_cpu (Request, cpu_t cpu)
{
    // Send IPI to specific CPU for reschedule or kernel entry
    send_ipi (cpu, 0);
}

void Interrupt::send_exc (Request)
{
    // Send IPI to all other CPUs
    // For single-core QEMU, this is a no-op
}

