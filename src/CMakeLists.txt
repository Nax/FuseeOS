include_directories(
  "${CMAKE_SOURCE_DIR}/include/fusee"
  $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/include/fusee/c++>
  "${CMAKE_SOURCE_DIR}/src"
)

# Initram/Image
set(INITRAM_CSV "${CMAKE_BINARY_DIR}/initram.csv")
set(IMAGE_CSV   "${CMAKE_BINARY_DIR}/image.csv")

set(IMAGE_LIST_FILES "" CACHE INTERNAL "")
set(IMAGE_LIST_TARGETS "" CACHE INTERNAL "")
set(IMAGE_DATA "" CACHE INTERNAL "")

set(INITRAM_LIST_FILES "" CACHE INTERNAL "")
set(INITRAM_LIST_TARGETS "" CACHE INTERNAL "")
set(INITRAM_DATA "" CACHE INTERNAL "")

function(install_image)
  cmake_parse_arguments(ARG "INITRAM" "FILE;TARGET;DESTINATION" "" ${ARGN})
  if (ARG_FILE)
    list(APPEND IMAGE_LIST_FILES "${ARG_FILE}")
    if (ARG_INITRAM)
      list(APPEND INITRAM_LIST_FILES "${ARG_FILE}")
    endif()
    get_filename_component(filename "${ARG_FILE}" NAME)
    set(entry "F,${ARG_DESTINATION}/${filename},${ARG_FILE}")
  else()
    list(APPEND IMAGE_LIST_TARGETS "${ARG_TARGET}")
    if (ARG_INITRAM)
      list(APPEND INITRAM_LIST_TARGETS "${ARG_TARGET}")
    endif()
    set(entry "F,${ARG_DESTINATION}/$<TARGET_FILE_NAME:${ARG_TARGET}>,$<TARGET_FILE:${ARG_TARGET}>")
  endif()
  list(APPEND IMAGE_DATA "${entry}")
  if (ARG_INITRAM)
    list(APPEND INITRAM_DATA "${entry}")
  endif()
  set(IMAGE_DATA "${IMAGE_DATA}" CACHE INTERNAL "")
  set(IMAGE_LIST_FILES "${IMAGE_LIST_FILES}" CACHE INTERNAL "")
  set(IMAGE_LIST_TARGETS "${IMAGE_LIST_TARGETS}" CACHE INTERNAL "")
  set(INITRAM_DATA "${INITRAM_DATA}" CACHE INTERNAL "")
  set(INITRAM_LIST_FILES "${INITRAM_LIST_FILES}" CACHE INTERNAL "")
  set(INITRAM_LIST_TARGETS "${INITRAM_LIST_TARGETS}" CACHE INTERNAL "")
endfunction()


set(FLAGS_COMMON        -fomit-frame-pointer)
set(FLAGS_FREESTANDING  ${FLAGS_COMMON} -ffreestanding -nostdlib -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics> $<$<COMPILE_LANGUAGE:CXX>:-std=c++14>)
set(FLAGS_BOOT          ${FLAGS_FREESTANDING} -mno-sse -mno-sse2)
set(FLAGS_KERNEL        ${FLAGS_FREESTANDING} $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit> -mcmodel=kernel -mno-red-zone)

add_subdirectory(libc)
add_subdirectory(libboot)
add_subdirectory(boot)
add_subdirectory(kernel)
add_subdirectory(user)

# Tools
set(MKINITRAM "${TOOLS}/mkinitram${EXE}")
set(MKVOLUME  "${TOOLS}/mkvolume${EXE}")
set(MKIMAGE   "${TOOLS}/mkimage${EXE}")

# Initram
list(JOIN INITRAM_DATA "\n" INITRAM_DATA_CSV)
file(GENERATE OUTPUT "${INITRAM_CSV}" CONTENT "${INITRAM_DATA_CSV}")
set(INITRAM "${CMAKE_BINARY_DIR}/initram")
add_custom_command(
  OUTPUT "${INITRAM}"
  COMMAND "${MKINITRAM}" "${INITRAM}" "${INITRAM_CSV}"
  DEPENDS
    "${MKINITRAM}"
    "${INITRAM_CSV}"
    "${INITRAM_LIST_FILES}"
    "${INITRAM_LIST_TARGETS}"
  VERBATIM
)
install_image(FILE "${INITRAM}" DESTINATION /boot)

# Volume
list(JOIN IMAGE_DATA "\n" IMAGE_DATA_CSV)
file(GENERATE OUTPUT "${IMAGE_CSV}" CONTENT "${IMAGE_DATA_CSV}")
set(VOLUME "${CMAKE_BINARY_DIR}/FuseeOS.vol")
add_custom_command(
  OUTPUT "${VOLUME}"
  COMMAND "${MKVOLUME}" "${VOLUME}" 67108864 "$<TARGET_FILE:bootloader>" "${IMAGE_CSV}"
  DEPENDS
    "${MKVOLUME}"
    "${IMAGE_CSV}"
    "${IMAGE_LIST_FILES}"
    "${IMAGE_LIST_TARGETS}"
    bootloader
  VERBATIM
)

# Image
set(IMAGE "${CMAKE_BINARY_DIR}/FuseeOS.img")
add_custom_command(
  OUTPUT "${IMAGE}"
  COMMAND "${MKIMAGE}" "${IMAGE}" "$<TARGET_FILE:mbr>" "${VOLUME}"
  DEPENDS
    "${MKIMAGE}"
    "${VOLUME}"
    mbr
  VERBATIM
)
add_custom_target(fusee_image ALL DEPENDS "${IMAGE}")
install(FILES "${IMAGE}" DESTINATION .)
