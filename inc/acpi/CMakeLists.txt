# inc/acpi/CMakeLists.txt — ACPI common headers

target_sources(nova PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_gas.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_dbg2.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_facs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_fadt.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_hest.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_madt.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_mcfg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_rsdp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_spcr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_srat.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_tpm2.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_xsdt.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console_fbuf.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uefi.hpp
)
