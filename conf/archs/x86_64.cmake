# conf/archs/x86_64.cmake — x86_64 architecture configuration

set(NOVA_ARCH_FLAGS
    -Wa,--divide,--noexecstack
    -march=x86-64-v2
    -mcmodel=kernel
    -mgeneral-regs-only
    -mno-red-zone
)

set(NOVA_ARCH_DEFINES "")
set(NOVA_ARCH_WFLAGS  "")
set(NOVA_ARCH_LFLAGS  "")
set(NOVA_OUTPUT_NAME  "${NOVA_ARCH}-nova")
