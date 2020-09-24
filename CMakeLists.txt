cmake_minimum_required(VERSION 3.10)

option(SUB_BUILD OFF)

if (SUB_BUILD)
    project(FuseeOS_SubBuild C CXX ASM)
    add_subdirectory(src)
else()
    project(FuseeOS)
    include(ExternalProject)
    ExternalProject_Add(ep_build
        SOURCE_DIR "${CMAKE_SOURCE_DIR}"
        INSTALL_DIR "${CMAKE_BINARY_DIR}/tree"
        BINARY_DIR "${CMAKE_BINARY_DIR}/src"
        CMAKE_ARGS
            -DSUB_BUILD=ON
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/CrossToolchain.cmake
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    )
    ExternalProject_Add_Step(ep_build reconfigure
        DEPENDEES download
        DEPENDERS configure
        ALWAYS 1
    )
    add_subdirectory(tools)
    set(IMAGE   "${CMAKE_BINARY_DIR}/fuseeOS.img")
    set(VOLUME  "${CMAKE_BINARY_DIR}/fuseeOS.vol")
    add_custom_command(
        OUTPUT "${VOLUME}"
        COMMAND mkvolume "${VOLUME}" 41943040 "${CMAKE_BINARY_DIR}/tree/boot/boot"
        DEPENDS
            ep_build
            mkvolume
            "${CMAKE_BINARY_DIR}/tree/boot/boot"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${IMAGE}"
        COMMAND mkimage "${IMAGE}" "${CMAKE_BINARY_DIR}/tree/boot/mbr" "${VOLUME}"
        DEPENDS
            ep_build
            mkimage
            "${CMAKE_BINARY_DIR}/tree/boot/mbr"
            "${VOLUME}"
        VERBATIM
    )
    add_custom_target(
        fuseeOS ALL
        DEPENDS "${IMAGE}"
    )
endif()
