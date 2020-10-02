file(GLOB_RECURSE SOURCES "*.c")

add_library(libc STATIC "${SOURCES}")
set_target_properties(libc PROPERTIES OUTPUT_NAME "c")

add_library(libc_kernel STATIC "${SOURCES}")
set_target_properties(libc_kernel PROPERTIES OUTPUT_NAME "c_kernel")
target_compile_options(libc_kernel PRIVATE -ffreestanding -nostdlib -mcmodel=kernel -mno-red-zone)
target_compile_definitions(libc_kernel PRIVATE __LIBC_MINIMAL__=1)

add_library(libc_boot STATIC "${SOURCES}")
set_target_properties(libc_boot PROPERTIES OUTPUT_NAME "c_boot")
target_compile_options(libc_boot PRIVATE -ffreestanding -nostdlib -mno-sse -mno-sse2)
target_compile_definitions(libc_boot PRIVATE __LIBC_MINIMAL__=1)
