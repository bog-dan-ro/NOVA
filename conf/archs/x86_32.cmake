# conf/archs/x86_32.cmake — x86_32 architecture configuration

set(NOVA_ARCH_FLAGS
    -Wa,--divide,--noexecstack
    -march=i486
    -m32
    -mgeneral-regs-only
    -mno-red-zone
)

# Bootstrap-specific flags
set(NOVA_BSP_ARCH_FLAGS
    -Wa,--divide,--noexecstack
    -march=i486
    -m32
    -mgeneral-regs-only
    -mno-red-zone
)

set(NOVA_ARCH_DEFINES "")
set(NOVA_ARCH_WFLAGS  "")
set(NOVA_ARCH_LFLAGS  "")
set(NOVA_OUTPUT_NAME  "${NOVA_ARCH}-nova")
