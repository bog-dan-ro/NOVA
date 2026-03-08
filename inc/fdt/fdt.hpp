/*
 * Assertions
 *
 * Copyright (C) 2026 BogDan Vatra <bogdan@kde.org>
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

#include <endian.hpp>
#include <std.hpp>
#include <string.hpp>
#include <types.hpp>

using dtptr_t = const uint32_t *;
struct PropertyInfo
{
        string_view name;
        const char *value_start = nullptr;
        uint32_t    value_length;
        uint32_t    address_cells;
        uint32_t    size_cells;
};

template<typename F>
concept RsvMapCallback = requires (F fn, uintptr_t addr, size_t size) {
    { fn (addr, size) } -> std::same_as<bool>;
};

template<typename F>
concept NodeCallback = requires (F fn, string_view name, const PropertyInfo *pi) {
    { fn (name, pi) } -> std::same_as<void>;
};

template<typename Type = uint64_t>
constexpr Type sized_value (dtptr_t &ptr, uint32_t size)
{
    switch (size) {
    case 1:
        return Aligned_be (*ptr++);
    case 2: {
        uint64_t hi { static_cast<uint64_t> (Aligned_be<uint32_t> (*ptr++)) };
        uint64_t lo { static_cast<uint64_t> (Aligned_be<uint32_t> (*ptr++)) };
        return (hi << 32) | lo;
    }
    default: {
        ptr += size - 2;
        uint64_t hi { static_cast<uint64_t> (Aligned_be<uint32_t> (*ptr++)) };
        uint64_t lo { static_cast<uint64_t> (Aligned_be<uint32_t> (*ptr++)) };
        return (hi << 32) | lo;
    }
    }
}
template<typename Type = uint64_t>
class RegValue
{
        RegValue() = default;

    public:
        Type                      address;
        Type                      size;
        static constexpr RegValue fromProperty (const PropertyInfo *property)
        {
            RegValue res;
            if (property->value_length != sizeof (uint32_t) * (property->address_cells + property->size_cells) || !property->address_cells || !property->size_cells || !property->value_start) {
                res.address = 0;
                res.size    = 0;
                return res;
            }
            auto ptr    = reinterpret_cast<dtptr_t> (static_cast<const void *> (property->value_start));
            res.address = sized_value (ptr, property->address_cells);
            res.size    = sized_value (ptr, property->size_cells);
            return res;
        }
};

class Fdt
{
    public:
        static constexpr uint32_t FDT_VERSION { 17 };

        // https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html#header
        struct Header
        {
                Aligned_be<uint32_t> const magic; // 0xd00dfeed
                Aligned_be<uint32_t> const totalsize;
                Aligned_be<uint32_t> const off_dt_struct;
                Aligned_be<uint32_t> const off_dt_strings;
                Aligned_be<uint32_t> const off_mem_rsvmap;
                Aligned_be<uint32_t> const version;
                Aligned_be<uint32_t> const last_comp_version;
                Aligned_be<uint32_t> const boot_cpuid_phys;
                Aligned_be<uint32_t> const size_dt_strings;
                Aligned_be<uint32_t> const size_dt_struct;
        };

        // https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html#format
        struct ReserveEntry
        {
                Aligned_be<uint64_t> address;
                Aligned_be<uint64_t> size;
        };

        // https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html#lexical-structure
        struct PropertyEntry
        {
                static const PropertyEntry *fromPtr (dtptr_t &ptr)
                {
                    auto res = reinterpret_cast<const PropertyEntry *> (ptr);
                    ptr += 2;
                    return res;
                }
                Aligned_be<uint32_t> len;
                Aligned_be<uint32_t> nameoff;
        };

        // https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html#lexical-structure
        enum Token : uint8_t
        {
            BeginNode = 0x01,
            EndNode   = 0x02,
            Property  = 0x03,
            Nop       = 0x04,
            End       = 0x9
        };

    public:
        explicit Fdt (const uint8_t *fdt_ptr) :
            _header (reinterpret_cast<const Header *> (static_cast<const void *> (fdt_ptr)))
        {
            if (!is_valid())
                return;
            _structs_start = reinterpret_cast<dtptr_t> (static_cast<const void *> (fdt_ptr + static_cast<uint32_t> (_header->off_dt_struct)));
            _structs_end   = reinterpret_cast<dtptr_t> (static_cast<const void *> (reinterpret_cast<const uint8_t *> (_structs_start) + static_cast<uint32_t> (_header->size_dt_struct)));
            _strings_start = reinterpret_cast<const char *> (fdt_ptr + _header->off_dt_strings);
            _strings_end   = _strings_start + _header->size_dt_strings;
            _end           = fdt_ptr + _header->totalsize;
        }

        inline bool          is_valid() const { return _header->magic == 0xd00dfeed && _header->last_comp_version <= FDT_VERSION; }
        inline const Header *header() const { return _header; }

        template<typename F>
        requires RsvMapCallback<F>
        bool for_each_rsvmap (F const &fn) const
        {
            if (_header->magic != 0xd00dfeed)
                return false;
            if (!_header->off_mem_rsvmap)
                return true;
            const char *const rsvmap = reinterpret_cast<const char *> (_header) + static_cast<uint32_t> (_header->off_mem_rsvmap);
            auto entry = reinterpret_cast<const ReserveEntry *> (static_cast<const void *> (rsvmap));
            while (reinterpret_cast<const uint8_t *> (entry) < _end && entry->address && entry->size) {
                if (!fn (entry->address, entry->size))
                    return false;
                ++entry;
            }
            return true;
        }

        template<typename F>
        requires NodeCallback<F>
        bool for_each_node (F const &fn)
        {
            auto s = _structs_start;
            for_each_node_impl (fn, s, ""_sv, 2, 2);
            return _ended && s == _structs_end;
        }

    private:
        template<typename F>
        requires NodeCallback<F>
        void for_each_node_impl (F const &fn, dtptr_t &pos, string_view nodeName, uint32_t address_cells, uint32_t size_cells)
        {
            uint32_t node_address_cells = 2;
            uint32_t node_size_cells    = 2;
            while (!_ended && pos < _structs_end) {
                switch (Aligned_be<uint32_t> (*pos++)) {
                case Token::BeginNode: {
                    string_view name { reinterpret_cast<const char *> (pos) };
                    fn (name, nullptr);
                    for_each_node_impl (fn, pos, name, node_address_cells, node_size_cells);
                } break;
                case Token::EndNode:
                    return;
                case Token::Nop:
                    break;
                case Token::End:
                    _ended = true;
                    break;
                case Token::Property: {
                    const auto        entry = PropertyEntry::fromPtr (pos);
                    const string_view name { _strings_start + entry->nameoff };
                    if (name == "#address-cells"_sv)
                        node_address_cells = Aligned_be<uint32_t> (*pos);
                    else if (name == "#size-cells"_sv)
                        node_address_cells = Aligned_be<uint32_t> (*pos);
                    PropertyInfo info { .name = name, .value_start = reinterpret_cast<const char *> (pos), .value_length = entry->len, .address_cells = address_cells, .size_cells = size_cells };
                    fn (nodeName, &info);
                    pos += (((info.value_length + 3) & ~3) >> 2);
                } break;
                }
            }
        }

    private:
        const Header  *_header;
        dtptr_t        _structs_start = nullptr;
        dtptr_t        _structs_end   = nullptr;
        const char    *_strings_start = nullptr;
        const char    *_strings_end   = nullptr;
        const uint8_t *_end           = nullptr;
        bool           _ended         = false;
};

/*
constexpr void addMemoryRegion(Hw::Memory_region region, Hw::Memory_region_array &array, const Hw::Memory_region_array &reserved)
{
	if (!region.size)
		return;

	for (const auto m : array) {
		if (m.contains(region))
			return;
	}

	bool changed = false;
	for (auto res : reserved) {
		if (res.contains(region))
			return;

		if (region.intersects(res)) {
			res.base = max(res.base, region.base);
			res.size = min(res.end(), region.end()) - res.base;
			if (region.base < res.base)
				addMemoryRegion({region.base, res.base - region.base}, array, reserved);
			if (region.end() > res.end())
				addMemoryRegion({res.end(), region.end() - res.end()}, array, reserved);
			changed = true;
		}
	}
	if (!changed)
		array.add(region);
}
*/
