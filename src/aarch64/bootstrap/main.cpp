// NOVA_HYP_ELF_PATH must contain the abs path to the nova hypervisor

extern const char elf_start[];
extern const char elf_end;

__asm__(
 ".section \".rodata\", \"a\", @progbits\n"
 ".balign 4096\n"
 "elf_start:\n"
 ".incbin \"" NOVA_HYP_ELF_PATH "\"\n"
 "elf_end:\n"
 ".previous\n"
);

extern "C" void start() {

    // stub - to be implemented

    for (;;)
        ;
}
