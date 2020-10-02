file(GLOB_RECURSE SOURCES "*.c" "*.cpp" "*.S")

add_library(libboot_kernel STATIC "${SOURCES}")
set_target_properties(libboot_kernel PROPERTIES OUTPUT_NAME "boot_kernel")
target_compile_options(libboot_kernel PRIVATE ${FLAGS_KERNEL})

add_library(libboot_bootloader STATIC "${SOURCES}")
set_target_properties(libboot_bootloader PROPERTIES OUTPUT_NAME "boot_bootloader")
target_compile_options(libboot_bootloader PRIVATE ${FLAGS_BOOT})
