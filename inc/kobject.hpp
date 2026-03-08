/*
 * Kernel Object
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

#include "macros.hpp"
#include "rcu.hpp"
#include "refcnt.hpp"
#include "slab.hpp"

/*
 * Base class for all NOVA kernel objects (Pd, Ec, Sc, Pt, Sm, Dc).
 *
 * Combines reference counting (Refcnt) with RCU-deferred reclamation
 * (Rcu::Element). The mandatory 64-byte alignment ensures the low 6 bits of
 * any Kobject pointer are always zero, which Capability uses to encode
 * permission bits in those bits (see Capability::pmask).
 *
 * Subclasses are allocated from per-PD Slab_cache instances and freed via
 * destroy() -> RCU grace period -> collect().
 */
class Kobject : public Refcnt, public Rcu::Element
{
    friend class Capability;

    public:
        // Minimum alignment of Kobject subclasses; equals the number of bits
        // available for capability permission encoding in a pointer.
        static constexpr auto alignment { BIT (6) };

        enum class Type : uint8_t
        {
            PD,             // Protection Domain
            EC,             // Execution Context
            SC,             // Scheduling Context
            PT,             // Portal
            SM,             // Semaphore
            DC,             // Device Context
        };

        enum class Subtype : uint8_t
        {
            NONE            = 0,

            // PD Subtypes
            PD              = 0,
            OBJ             = 1,
            HST             = 2,
            GST             = 3,
            DMA             = 4,
            PIO             = 5,
            MSR             = 6,

            // EC Subtypes
            EC_LOCAL        = 0,
            EC_GLOBAL       = 1,
            EC_VCPU_REAL    = 2,
            EC_VCPU_OFFS    = 3,

            // SM Subtypes
            SM_REG          = 0,
            SM_INT          = 1,
        };

        Type    const   type;
        Subtype const   subtype;

    protected:
        explicit Kobject (Type t, Subtype s = Subtype::NONE) : type { t }, subtype { s } {}

        // When the last reference is dropped, submit to the RCU queue so destroy()
        // is called only after all CPUs have passed through a quiescent state.
        void retire() override final { rcu_submit(); }

        [[nodiscard]] static void *operator new (size_t, Slab_cache &cache) noexcept
        {
            return cache.alloc();
        }

        static void operator delete (void *ptr, Slab_cache &cache)
        {
            cache.free (ptr);
        }
};
