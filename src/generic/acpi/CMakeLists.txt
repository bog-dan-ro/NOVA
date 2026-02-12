# src/generic/acpi/CMakeLists.txt — ACPI feature sources (shared)

target_sources(nova PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_dbg2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_facs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_mcfg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_spcr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_tpm2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/acpi_table_xsdt.cpp
)
