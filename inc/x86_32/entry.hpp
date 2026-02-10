/*
 * Entry/Exit Functions: x86_32
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

#ifdef __ASSEMBLER__
#define PREG(X)         %X
#else
#define PREG(X)         %%X
#endif

#define SAVE_GPR        push    PREG(edi);  \
                        push    PREG(esi);  \
                        push    PREG(ebp);  \
                        push    PREG(ebx);  \
                        push    PREG(edx);  \
                        push    PREG(ecx);  \
                        push    PREG(eax);

#define LOAD_GPR        pop     PREG(eax);  \
                        pop     PREG(ecx);  \
                        pop     PREG(edx);  \
                        pop     PREG(ebx);  \
                        pop     PREG(ebp);  \
                        pop     PREG(esi);  \
                        pop     PREG(edi);

#define IRET            lea     2 * __SIZEOF_POINTER__(PREG(esp)), PREG(esp); \
                        iret

#define OFS_VEC         (__SIZEOF_POINTER__ * 8)
#define OFS_CS          (__SIZEOF_POINTER__ * 10)
