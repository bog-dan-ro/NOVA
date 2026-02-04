/*
 * Memory Attributes
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
#include "types.hpp"

// Memory attribute type for RISC-V (used in PTEs with Svpbmt extension)
class Memattr
{
    public:
        // Output physical address bits (Sv39 = 56 physical bits)
        static constexpr unsigned obits { 56 };

        // Key ID max (no memory encryption in RISC-V yet)
        static constexpr unsigned kimax { 0 };

        enum class Type : unsigned
        {
            PMA = 0,    // Use Physical Memory Attributes
            NC  = 1,    // Non-cacheable, idempotent, weakly-ordered (IO)
            IO  = 2,    // Non-cacheable, non-idempotent, strongly-ordered
        };

    private:
        Type type;

    public:
        constexpr Memattr() : type { Type::PMA } {}
        constexpr Memattr (Type t) : type { t } {}
        constexpr explicit Memattr (uint32_t v) : type { static_cast<Type>(v & 0x3) } {}

        constexpr auto value() const { return static_cast<unsigned>(type); }

        constexpr bool valid() const { return true; }  // All values are valid for RISC-V

        constexpr bool operator== (Memattr const &x) const { return type == x.type; }

        // Memory types
        static constexpr Memattr ram()  { return Memattr { Type::PMA }; }
        static constexpr Memattr dev()  { return Memattr { Type::IO }; }
        static constexpr Memattr gfx()  { return Memattr { Type::NC }; }  // Graphics framebuffer uses non-cacheable
};
