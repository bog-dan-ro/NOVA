# inc/CMakeLists.txt — Common headers and architecture/feature dispatch

# ---------------------------------------------------------------------------
# Common headers (shared by all architectures)
# ---------------------------------------------------------------------------

target_sources(nova PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/alignment.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/assert.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/atomic.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bitmap.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bits.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/buddy.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/capability.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/checksum.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cmdline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console_mbuf.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console_uart.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dc.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/debug.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ec.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/elf.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/endian.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hash.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hazard.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hip.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/initprio.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kobject.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/list.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lock_guard.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/macros.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mmio.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mtd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/multiboot.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/paging.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/patch.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pci.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pt.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ptab.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rcu.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/refcnt.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sc.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scheduler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdid.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/signature.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/slab.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sm.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/space.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/space_mem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/space_obj.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/status.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stc.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/std.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stdio.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/string.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/syscall.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeout.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeout_budget.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeout_hypercall.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utcb.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uuid.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wait.hpp
)

# ---------------------------------------------------------------------------
# Architecture-specific headers
# ---------------------------------------------------------------------------

add_subdirectory(${NOVA_ARCH})

# ---------------------------------------------------------------------------
# Feature-specific headers
# ---------------------------------------------------------------------------

foreach(_feature IN LISTS NOVA_FEATURES)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_feature}")
        add_subdirectory("${_feature}")
    endif()

    set(_arch "${CMAKE_CURRENT_SOURCE_DIR}/${NOVA_ARCH}/${_feature}")
    if(IS_DIRECTORY "${_arch}")
        add_subdirectory("${NOVA_ARCH}/${_feature}")
    endif()
endforeach()
