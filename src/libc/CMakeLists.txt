file(GLOB_RECURSE SOURCES "*.c" "*.S")

set(CRT_DIR "${CMAKE_BINARY_DIR}/crt")
set(CRT0_SRC "${CMAKE_CURRENT_SOURCE_DIR}/crt0.S")
set(CRT0 "${CRT_DIR}/crt0.o")
make_directory("${CRT_DIR}")

list(REMOVE_ITEM SOURCES "${CRT0_SRC}")

add_library(libc STATIC "${SOURCES}")
set_target_properties(libc PROPERTIES OUTPUT_NAME "c")

add_custom_command(
    OUTPUT "${CRT0}"
    COMMAND ${CMAKE_C_COMPILER} -c "${CMAKE_CURRENT_SOURCE_DIR}/crt0.S" -o "${CRT0}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/crt0.S"
    COMMENT "Building crt0.o"
    VERBATIM
)
add_custom_target(libc_crt0 DEPENDS "${CRT0}")
add_dependencies(libc libc_crt0)
set_target_properties(libc PROPERTIES INTERFACE_LINK_LIBRARIES "${CRT0}")

add_library(libc_kernel STATIC "${SOURCES}")
set_target_properties(libc_kernel PROPERTIES OUTPUT_NAME "c_kernel")
target_compile_options(libc_kernel PRIVATE -ffreestanding -nostdlib -mcmodel=kernel -mno-red-zone)
target_compile_definitions(libc_kernel PRIVATE __LIBC_MINIMAL__=1 PUBLIC _KERNEL=1)

add_library(libc_boot STATIC "${SOURCES}")
set_target_properties(libc_boot PROPERTIES OUTPUT_NAME "c_boot")
target_compile_options(libc_boot PRIVATE -ffreestanding -nostdlib -mno-sse -mno-sse2)
target_compile_definitions(libc_boot PRIVATE __LIBC_MINIMAL__=1)
