cmake_minimum_required(VERSION 3.10)
if (SUPERBUILD_TOOLS)
  project(FuseeOS-Tools C CXX)
  add_subdirectory(tools)
elseif (SUPERBUILD_SRC)
  project(FuseeOS C CXX ASM)
  add_subdirectory(src)
else()
  project(FuseeOS-Superbuild NONE)
  include(ExternalProject)

  ExternalProject_Add(
    ep_fusee
    SOURCE_DIR  "${CMAKE_SOURCE_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    BINARY_DIR  "${CMAKE_BINARY_DIR}/ep/src"
    CMAKE_ARGS
      -DSUPERBUILD_SRC=ON
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_BUILD_TYPE=$<CONFIG>
      "-DTOOLS=${CMAKE_BINARY_DIR}/tools"
      "-DEXE=${CMAKE_EXECUTABLE_SUFFIX}"
      "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/CrossGNUToolchain.cmake"
  )
  ExternalProject_Add_Step(ep_fusee reconfigure
    DEPENDEES download
    DEPENDERS configure
    ALWAYS 1
  )

  ExternalProject_Add(
    ep_tools
    SOURCE_DIR  "${CMAKE_SOURCE_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    BINARY_DIR  "${CMAKE_BINARY_DIR}/ep/tools"
    CMAKE_ARGS
      -DSUPERBUILD_TOOLS=ON
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_BUILD_TYPE=$<CONFIG>
  )
  ExternalProject_Add_Step(ep_tools reconfigure
    DEPENDEES download
    DEPENDERS configure
    ALWAYS 1
  )

  add_dependencies(ep_fusee ep_tools)
endif()

if (__UNUSED__)
  add_subdirectory(src)
elseif(__UNUSED2__)
  add_subdirectory(tools)

  set(FUSEE_INSTALL   "${CMAKE_BINARY_DIR}/fusee-install")
  set(TREE            "${CMAKE_BINARY_DIR}/fusee-install/root")
  set(INITRAM         "${TREE}/boot/initram")
  set(FUSEE_STAMP     "${FUSEE_INSTALL}/stamp")
  set(VOLUME          "${CMAKE_BINARY_DIR}/fuseeOS.vol")
  set(IMAGE           "${CMAKE_BINARY_DIR}/fuseeOS.img")
  make_directory("${TREE}")

  # Build FuseeOS
  include(ExternalProject)
  ExternalProject_Add(
      ep_fusee
      SOURCE_DIR  "${CMAKE_SOURCE_DIR}"
      INSTALL_DIR "${FUSEE_INSTALL}"
      BINARY_DIR  "${CMAKE_BINARY_DIR}/superbuild"
      CMAKE_ARGS
          -DIN_SUPERBUILD=ON
          -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
          -DCMAKE_BUILD_TYPE=$<CONFIG>
          "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/CrossGNUToolchain.cmake"
  )
  ExternalProject_Add_Step(ep_fusee reconfigure
      DEPENDEES download
      DEPENDERS configure
      ALWAYS 1
  )

  # Assemble the FuseeOS ISO
  add_custom_command(
      OUTPUT "${IMAGE}"
      COMMAND mkinitram "${INITRAM}" "${CMAKE_SOURCE_DIR}/data/initram.txt" "${TREE}"
      COMMAND mkvolume "${VOLUME}" 67108864 "${FUSEE_INSTALL}/bootloader" "${CMAKE_SOURCE_DIR}/data/image.txt" "${TREE}"
      COMMAND mkimage "${IMAGE}" "${FUSEE_INSTALL}/mbr" "${VOLUME}"
      DEPENDS
          ep_fusee
          mkinitram
          mkvolume
          mkimage
          "${TREE}"
          "${CMAKE_SOURCE_DIR}/data/initram.txt"
          "${CMAKE_SOURCE_DIR}/data/image.txt"
      COMMENT
          "Generating the FuseeOS image"
      VERBATIM
  )
  add_custom_target(
      fuseeOS ALL
      DEPENDS "${IMAGE}"
  )
  add_dependencies(fuseeOS ep_fusee)
endif()
