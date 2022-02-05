include_directories(
  "${CMAKE_SOURCE_DIR}/include/fusee"
  $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/include/fusee/c++>
  "${CMAKE_SOURCE_DIR}/src"
)

# Initram/Image
set(INITRAM_TXT "${CMAKE_BINARY_DIR}/initram.txt")
set(IMAGE_TXT   "${CMAKE_BINARY_DIR}/image.txt")

function(install_image)
  cmake_parse_arguments(IMAGE "INITRAM" "FILE;TARGET;DESTINATION" "" ${ARGN})
  if (IMAGE_FILE)
    get_filename_component(filename "${IMAGE_FILE}" NAME)
    set(entry "F ${IMAGE_DESTINATION}/${NAME} ${IMAGE_FILE}\n")
  else()
    set(entry "F ${IMAGE_DESTINATION}/$<TARGET_FILE_NAME:${IMAGE_TARGET}> $<TARGET_FILE:${IMAGE_TARGET}>\n")
  endif()
  set_property(GLOBAL APPEND_STRING PROPERTY IMAGE_LIST "${entry}")
  if (IMAGE_INITRAM)
    set_property(GLOBAL APPEND_STRING PROPERTY INITRAM_LIST "${entry}")
  endif()
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
get_property(INITRAM_LIST GLOBAL PROPERTY INITRAM_LIST)
file(GENERATE OUTPUT "${INITRAM_TXT}" CONTENT "${INITRAM_LIST}")
set(INITRAM "${CMAKE_BINARY_DIR}/initram")
add_custom_command(
  OUTPUT "${INITRAM}"
  COMMAND "${MKINITRAM}" "${INITRAM}" "${INITRAM_TXT}"
  DEPENDS
    "${MKINITRAM}"
    "${INITRAM_TXT}"
  VERBATIM
)
install_image(FILE "${INITRAM}" DESTINATION /boot)

# Volume
get_property(IMAGE_LIST GLOBAL PROPERTY IMAGE_LIST)
file(GENERATE OUTPUT "${IMAGE_TXT}" CONTENT "${IMAGE_LIST}")
set(VOLUME "${CMAKE_BINARY_DIR}/FuseeOS.vol")
add_custom_command(
  OUTPUT "${VOLUME}"
  COMMAND "${MKVOLUME}" "${VOLUME}" 67108864 "$<TARGET_FILE:bootloader>" "${IMAGE_TXT}"
  DEPENDS
    "${MKVOLUME}"
    "${IMAGE_TXT}"
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
