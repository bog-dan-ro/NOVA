/*
 * Console: NS16550 UART
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

#include "console.hpp"
#include "debug.hpp"
#include "sbi.hpp"

class Console_sbi final : private Console
{
    private:
        static Console_sbi sbi[];
        bool outc(char  c) final
        {
            return Sbi::putc(c).error == Sbi::SUCCESS;
        }

    protected:
        explicit Console_sbi()
            : Console{}
        {
        }
};



