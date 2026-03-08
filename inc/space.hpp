/*
 * Space
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
 * Copyright (C) 2019-2026 Udo Steinberg, BlueRock Security, Inc.
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

#include "pd.hpp"

/*
 * Base class for all NOVA address spaces (object, host, guest, DMA, PIO, MSR).
 *
 * A Space is a Kobject with Type::PD and one of the PD subtypes (OBJ, HST,
 * GST, DMA, PIO, MSR). Each space holds a reference to its owner PD.
 * The kernel's own spaces are rooted in Pd::nova and use the no-owner
 * constructor; all other spaces take an explicit Refptr<Pd>.
 *
 * mco and sbw are per-subclass constants exposed for HIP population:
 *   mco: mapping count order (log2 of slots per Captable/page-table level)
 *   sbw: selector bit width (total number of address/selector bits)
 */
class Space : public Kobject
{
    private:
        Refptr<Pd> const pd;

    protected:
        Space (Kobject::Subtype s) : Kobject { Kobject::Type::PD, s }, pd { &Pd::nova } {}

        Space (Kobject::Subtype s, Refptr<Pd> &ref_pd) : Kobject { Kobject::Type::PD, s }, pd { std::move (ref_pd) } {}

    public:
        // Architecturally supported spaces must override these defaults in the derived class
        static constexpr uint8_t mco { 0 };
        static constexpr uint8_t sbw { 0 };

        Pd *get_pd() const { return pd; }
};
