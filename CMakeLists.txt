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
            -DCMAKE_BUILD_TYPE=$<CONFIG>
    )
    ExternalProject_Add_Step(ep_build reconfigure
        DEPENDEES download
        DEPENDERS configure
        ALWAYS 1
    )
    include_directories(include)
    add_subdirectory(tools)

    set(TREE "${CMAKE_BINARY_DIR}/tree")
    file(GLOB_RECURSE TREE_FILES "${TREE}/*")
    set(IMAGE   "${CMAKE_BINARY_DIR}/fuseeOS.img")
    set(VOLUME  "${CMAKE_BINARY_DIR}/fuseeOS.vol")
    set(INITRAM "${TREE}/boot/initram")
    list(REMOVE_ITEM TREE_FILES ${INITRAM})

    add_custom_command(
        OUTPUT "${INITRAM}"
        COMMAND mkinitram "${INITRAM}" "${CMAKE_SOURCE_DIR}/data/initram.txt" "${TREE}"
        DEPENDS
            ep_build
            mkinitram
            "${CMAKE_SOURCE_DIR}/data/initram.txt"
            "${TREE}"
            ${TREE_FILES}
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${VOLUME}"
        COMMAND mkvolume "${VOLUME}" 41943040 "${TREE}/boot/boot" "${CMAKE_SOURCE_DIR}/data/image.txt" "${TREE}"
        DEPENDS
            ep_build
            mkvolume
            "${INITRAM}"
            "${CMAKE_SOURCE_DIR}/data/image.txt"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${IMAGE}"
        COMMAND mkimage "${IMAGE}" "${TREE}/boot/mbr" "${VOLUME}"
        DEPENDS
            ep_build
            mkimage
            "${TREE}/boot/mbr"
            "${VOLUME}"
        VERBATIM
    )
    add_custom_target(
        fuseeOS ALL
        DEPENDS "${IMAGE}"
    )
endif()
