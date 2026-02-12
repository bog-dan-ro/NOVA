# conf/boards/x86_64/pc.cmake — x86_64 PC board configuration

set(NOVA_FEATURES acpi uefi)

# Board-specific sources (append to NOVA_BOARD_SOURCES if needed)
# list(APPEND NOVA_BOARD_SOURCES ...)

# Override linker script (uncomment to override the default)
# set(NOVA_LINKER_SCRIPT "${nova_SOURCE_DIR}/path/to/custom.ld")
