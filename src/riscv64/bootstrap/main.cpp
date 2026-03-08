#include "bootinfo.hpp"
#include "fdt/fdt.hpp"
#include "sbi.hpp"
#include "types.hpp"

// NOVA_HYP_ELF_PATH must contain the abs path to the nova hypervisor
extern const char elf_start[];
extern const char elf_end;

__asm__ (
    ".section \".rodata\", \"a\", @progbits\n"
    ".balign 4096\n"
    "elf_start:\n"
    ".incbin \"" NOVA_HYP_ELF_PATH "\"\n"
    "elf_end:\n"
    ".previous\n");

// A global variable (goes in .data section)
int boot_count = 1;

// Uninitialized global variable (goes in .bss section)
int status_flag;

[[noreturn]] static void raise() { for (;;); }

BootInfo *info;

constexpr void addMemoryRegion (Region region, Fdt &fdt)
{
    if (!region.size)
        return;

    for (const auto m : RegionIterator { info->ram_regions }) {
        if (m.contains (region))
            return;
    }

    bool addRegion = fdt.for_each_rsvmap ([&] (uintptr_t addr, size_t size) -> bool {
        Region res { addr, size };
        if (res.contains (region))
            return false;
        if (region.intersects (res)) {
            res.addr = max (res.addr, region.addr);
            res.size = min (res.end(), region.end()) - res.addr;
            if (region.addr < res.addr)
                addMemoryRegion ({ region.addr, res.addr - region.addr }, fdt);
            if (region.end() > res.end())
                addMemoryRegion ({ res.end(), region.end() - res.end() }, fdt);
            return false;
        }
        return true;
    });
    if (addRegion) {
        auto ptr = info->ram_regions;
        while (ptr->addr || ptr->size)
            ++ptr;
        *ptr++ = region;
        *ptr   = Region {};
    }
}

extern "C" [[noreturn]] void start (uintptr_t cpu, const uint8_t *fdt_ptr)
{
    Fdt fdt { fdt_ptr };
    if (!fdt.is_valid() || cpu != fdt.header()->boot_cpuid_phys)
        raise();

    auto res = fdt.for_each_node ([&] (string_view nodeName, const PropertyInfo *property) {
        if (nodeName.starts_with ("cpus"_sv) && property && property->name == "timebase-frequency"_sv) {
            info->timer_hz = Aligned_be<uint32_t> (*reinterpret_cast<const uint32_t *> (static_cast<const void *> (property->value_start)));
        }
        else if (nodeName.starts_with ("cpu@"_sv)) {
            if (!property) {
                ++info->cpu_count;
            }
            else {
                if (!info->timer_hz && property->name == "timebase-frequency"_sv)
                    info->timer_hz = Aligned_be<uint32_t> (*reinterpret_cast<const uint32_t *> (static_cast<const void *> (property->value_start)));
            }
        }
        else if (nodeName.starts_with ("memory@"_sv) && property && property->name == "reg"_sv) {
            auto reg = RegValue<uint64_t>::fromProperty (property);
            addMemoryRegion ({ reg.address, reg.size }, fdt);
        }
        else if (nodeName.starts_with ("interrupt-controller@"_sv) && property && property->name == "reg"_sv) {
            auto reg = RegValue<uint64_t>::fromProperty (property);
            addMemoryRegion ({ reg.address, reg.size }, fdt);
            info->plic.addr = reg.address;
            info->plic.size = reg.size;
        } else if (nodeName == "chosen"_sv && property && property->name == "bootargs"_sv) {
            info->boot_args = uintptr_t(property->value_start);
        }
    });

    if (!res)
        raise();

    raise();
}
