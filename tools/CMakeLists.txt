if (WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
endif()

include_directories("${CMAKE_SOURCE_DIR}/include")
add_executable(mkimage  mkimage.c)
add_executable(mkvolume mkvolume.c)
add_executable(mkinitram mkinitram.c)

install(TARGETS mkimage mkvolume mkinitram RUNTIME DESTINATION tools)
