/*
 * Interrupt Handling
 *
 * Copyright (C) 2019-2020 Udo Steinberg, BedRock Systems, Inc.
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

#include "assert.hpp"
#include "counter.hpp"
#include "gicc.hpp"
#include "gicd.hpp"
#include "gicr.hpp"
#include "interrupt.hpp"
#include "sc.hpp"
#include "sm.hpp"
#include "smmu.hpp"
#include "stdio.hpp"
#include "timer.hpp"

Interrupt Interrupt::int_table[NUM_SPI];

unsigned Interrupt::count()
{
    return Gicd::ints - BASE_SPI;
}

void Interrupt::init()
{
    for (unsigned spi = 0; spi < sizeof (int_table) / sizeof (*int_table); spi++)
        if (!Smmu::using_spi (spi))
            int_table[spi].sm = Sm::create (0, spi);
}

Event::Selector Interrupt::handle_sgi (uint32 val, bool)
{
    auto sgi = Intid::to_sgi (val & 0x3ff);

    assert (sgi < NUM_SGI);

    Counter::req[sgi].inc();

    Gicc::eoi (val);

    switch (sgi) {
        case Request::RRQ: Sc::rrq_handler(); break;
        case Request::RKE: break;
    }

    Gicc::dir (val);

    return Event::Selector::NONE;
}

Event::Selector Interrupt::handle_ppi (uint32 val, bool vcpu)
{
    auto ppi = Intid::to_ppi (val & 0x3ff);

    assert (ppi < NUM_PPI);

    Counter::loc[ppi].inc();

    Gicc::eoi (val);

    Event::Selector evt = Event::Selector::NONE;

    if (ppi == Timer::ppi_el2_p)
        Timer::interrupt();

    else if (ppi == Timer::ppi_el1_v && vcpu)
        evt = Event::Selector::VTIMER;

    if (evt == Event::Selector::NONE)
        Gicc::dir (val);

    return evt;
}

Event::Selector Interrupt::handle_spi (uint32 val, bool)
{
    auto spi = Intid::to_spi (val & 0x3ff);

    assert (spi < NUM_SPI);

    Gicc::eoi (val);

    if (EXPECT_TRUE (int_table[spi].sm)) {

        if (!int_table[spi].gst)
            int_table[spi].dir = true;

        int_table[spi].sm->up();

    } else {

        Smmu::interrupt (spi);

        Gicc::dir (val);
    }

    return Event::Selector::NONE;
}

Event::Selector Interrupt::handler (bool vcpu)
{
    uint32 val = Gicc::ack(), i = val & 0x3ff;

    if (i < BASE_PPI)
        return handle_sgi (val, vcpu);

    if (i < BASE_SPI)
        return handle_ppi (val, vcpu);

    if (i < BASE_RSV)
        return handle_spi (val, vcpu);

    return Event::Selector::NONE;
}

void Interrupt::conf_sgi (unsigned sgi, bool msk)
{
    if (Gicd::arch < 3)
        Gicd::conf (sgi + BASE_SGI, false);
    else
        Gicr::conf (sgi + BASE_SGI, false);

    (Gicd::arch < 3 ? Gicd::mask : Gicr::mask) (sgi + BASE_SGI, msk);
}

void Interrupt::conf_ppi (unsigned ppi, bool msk, bool lvl)
{
    if (Gicd::arch < 3)
        Gicd::conf (ppi + BASE_PPI, lvl);
    else
        Gicr::conf (ppi + BASE_PPI, lvl);

    (Gicd::arch < 3 ? Gicd::mask : Gicr::mask) (ppi + BASE_PPI, msk);
}

void Interrupt::conf_spi (unsigned spi, unsigned cpu, bool msk, bool lvl)
{
    Gicd::conf (spi + BASE_SPI, lvl, cpu);
    Gicd::mask (spi + BASE_SPI, msk);
}

void Interrupt::configure (unsigned spi, unsigned flags, unsigned cpu, uint16, uint32 &msi_addr, uint16 &msi_data)
{
    bool msk = flags & BIT (0);
    bool lvl = flags & BIT (1);
    bool gst = flags & BIT (3);

    trace (TRACE_INTR, "INTR: %s: SPI:%03u CPU:%u %c%c%c", __func__, spi, cpu, msk ? 'M' : 'U', lvl ? 'L' : 'E', gst ? 'G' : 'H');

    int_table[spi].cpu = static_cast<uint16>(cpu);
    int_table[spi].gst = gst;

    conf_spi (spi, cpu, msk, lvl);

    msi_addr = msi_data = 0;
}

void Interrupt::deactivate (unsigned spi)
{
    if (int_table[spi].dir) {
        int_table[spi].dir = false;
        Gicc::dir (spi + BASE_SPI);
    }
}

void Interrupt::send_cpu (unsigned cpu, Request req)
{
    (Gicd::arch < 3 ? Gicd::send_sgi : Gicc::send_sgi) (cpu, req);
}
