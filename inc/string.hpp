/*
 * String Functions
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
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

#include "compiler.hpp"
#include "types.hpp"
#include "util.hpp"

extern "C" __attribute__((nonnull, used))
inline void *memcpy (void *d, void const *s, size_t n)
{
    auto dst { static_cast<char *>(d) };
    auto src { static_cast<char const *>(s) };

    while (n--)
        *dst++ = *src++;

    return d;
}

extern "C" __attribute__((nonnull, used))
inline void *memset (void *d, int c, size_t n)
{
    auto dst { static_cast<char *>(d) };

    while (n--)
        *dst++ = static_cast<char>(c);

    return d;
}

extern "C" __attribute__((nonnull))
inline int memcmp (void const *s1, void const *s2, size_t n)
{
    auto p1 { static_cast<unsigned char const *>(s1) };
    auto p2 { static_cast<unsigned char const *>(s2) };

    for (; n--; p1++, p2++)
        if (*p1 != *p2)
            return *p1 - *p2;

    return 0;
}


class string_view {
    const char *_string;
    uint32_t _size = 0;

public:
    static constexpr uint32_t  npos = uint32_t (-1);

public:
    constexpr string_view(const char *str)
        : _string(str)
        , _size(strlen(str))
    {}

    constexpr string_view(const char *str, uint32_t sz)
        : _string(str)
        , _size(sz)
    {}

    static constexpr uint32_t strlen(const char *s) {
        if (!s)
            return 0;
        uint32_t res = 0;
        while(*s++)
            ++res;
        return res;
    }

    constexpr void clear() { _string = nullptr, _size = 0; }
    constexpr const char *data() const {return _string;}
    constexpr uint32_t size() const {return _size;}
    constexpr uint32_t length() const {return _size;}
    constexpr bool empty() const {return _size == 0; }
    constexpr const char &operator[](uint32_t pos) const { return *(_string + pos); }
    constexpr const char &front() const {return *_string;}
    constexpr const char &back() const {return *(_string + _size - 1);}

    constexpr const char * begin() const { return _string; }
    constexpr const char * end() const { return _string + _size; }

    constexpr bool operator==(string_view str) const
    {
        if (_size != str._size)
            return false;
        auto size = _size;
        if (!size)
            return true;
        auto s1 = _string;
        auto s2 = str._string;
        while (--size && *s1 && *s1 == *s2)
        {
            ++s1;
            ++s2;
        }
        return *s1 == *s2;
    }

    constexpr bool operator!=(string_view str) const {
        return !operator ==(str);
    }

    constexpr string_view substr(uint32_t pos = 0, uint32_t count = npos) const
    {
        pos = min(pos, _size);
        count = min(count, _size - pos);
        return {_string + pos, count};
    }

    constexpr bool starts_with(string_view str) const
    {
        return _size >= str._size && substr(0, str.size()) == str;
    }
    constexpr bool ends_with(string_view str) const
    {
        return _size >= str._size && substr(_size - str._size) == str;
    }

    constexpr uint32_t find(string_view str, uint32_t pos = 0) const
    {
        if (str._size > _size)
            return npos;
        const auto sz = _size - str._size;
        for(; pos <= sz ; ++pos) {
            if (substr(pos, str._size) == str)
                return pos;
        }
        return npos;
    }

    constexpr uint32_t find(char ch, uint32_t pos = 0) const
    {
        for (; pos < _size; ++pos) {
            if (*(_string + pos) == ch)
                return pos;
        }
        return npos;
    }
};

constexpr string_view operator ""_sv(const char*str) {return {str};}
#ifdef __i386__
constexpr string_view operator ""_sv(const char*str, unsigned len) {return {str, uint32_t(len)};}
#else
constexpr string_view operator ""_sv(const char*str, unsigned long len) {return {str, uint32_t(len)};}
#endif
