set(SOURCES mbr.S mbr.ld)
add_executable(mbr ${SOURCES})
target_link_options(mbr PRIVATE -ffreestanding -nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/mbr.ld)
install(TARGETS mbr RUNTIME DESTINATION boot)
