file(GLOB_RECURSE SOURCES "*.S")
add_executable(boot ${SOURCES})
target_link_options(boot PRIVATE -ffreestanding -nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/boot.ld)
install(TARGETS boot RUNTIME DESTINATION boot)
