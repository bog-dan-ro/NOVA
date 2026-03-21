/*
 * Compiler Runtime Support Functions
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

#include "types.hpp"

extern "C" {

/*
 * Count leading zeros in a 64-bit value
 *
 * @param val   Value to count leading zeros in
 * @return      Number of leading zero bits (0-64)
 */
int __clzdi2 (uint64_t val)
{
    if (!val)
        return 64;

    int n { 0 };

    if (!(val & 0xFFFFFFFF00000000ULL)) { n += 32; val <<= 32; }
    if (!(val & 0xFFFF000000000000ULL)) { n += 16; val <<= 16; }
    if (!(val & 0xFF00000000000000ULL)) { n +=  8; val <<=  8; }
    if (!(val & 0xF000000000000000ULL)) { n +=  4; val <<=  4; }
    if (!(val & 0xC000000000000000ULL)) { n +=  2; val <<=  2; }
    if (!(val & 0x8000000000000000ULL)) { n +=  1; }

    return n;
}

/*
 * Count trailing zeros in a 64-bit value
 *
 * @param val   Value to count trailing zeros in
 * @return      Number of trailing zero bits (0-64)
 */
int __ctzdi2 (uint64_t val)
{
    if (!val)
        return 64;

    int n { 0 };

    if (!(val & 0x00000000FFFFFFFFULL)) { n += 32; val >>= 32; }
    if (!(val & 0x000000000000FFFFULL)) { n += 16; val >>= 16; }
    if (!(val & 0x00000000000000FFULL)) { n +=  8; val >>=  8; }
    if (!(val & 0x000000000000000FULL)) { n +=  4; val >>=  4; }
    if (!(val & 0x0000000000000003ULL)) { n +=  2; val >>=  2; }
    if (!(val & 0x0000000000000001ULL)) { n +=  1; }

    return n;
}

} // extern "C"
