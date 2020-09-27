cmake_minimum_required(VERSION 3.10)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/CrossToolchain.cmake")
project(FuseeOS C CXX ASM)

include(ExternalProject)
ExternalProject_Add(
    ep_tools
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    BINARY_DIR "${CMAKE_BINARY_DIR}/tools"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=$<CONFIG>
)
ExternalProject_Add_Step(ep_tools reconfigure
        DEPENDEES download
        DEPENDERS configure
        ALWAYS 1
)

add_compile_options(-Wall)
add_subdirectory(src)

set(MKINITRAM   "${CMAKE_BINARY_DIR}/bin/mkinitram")
set(MKVOLUME    "${CMAKE_BINARY_DIR}/bin/mkvolume")
set(MKIMAGE     "${CMAKE_BINARY_DIR}/bin/mkimage")

set(TREE    "${CMAKE_BINARY_DIR}/tree")
set(INITRAM "${TREE}/boot/initram")

set(VOLUME  "${CMAKE_BINARY_DIR}/fuseeOS.vol")
set(IMAGE   "${CMAKE_BINARY_DIR}/fuseeOS.img")

make_directory("${TREE}")
add_custom_command(
    OUTPUT "${IMAGE}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel> "${TREE}/boot/kernel"
    COMMAND ${MKINITRAM} ${INITRAM} "${CMAKE_SOURCE_DIR}/data/initram.txt" "${TREE}"
    COMMAND ${MKVOLUME} ${VOLUME} 67108864 $<TARGET_FILE:bootloader> "${CMAKE_SOURCE_DIR}/data/image.txt" "${TREE}"
    COMMAND ${MKIMAGE} ${IMAGE} $<TARGET_FILE:mbr> "${VOLUME}"
    DEPENDS
        ep_tools
        kernel
        bootloader
        mbr
        "${CMAKE_SOURCE_DIR}/data/initram.txt"
        "${CMAKE_SOURCE_DIR}/data/image.txt"
    VERBATIM
)
add_custom_target(
    fuseeOS ALL
    DEPENDS "${IMAGE}"
)
