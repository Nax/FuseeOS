if (WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
endif()

include_directories("${CMAKE_SOURCE_DIR}/include")

add_library(spec_parser STATIC spec_parser.c)

add_executable(mkimage  mkimage.c)
add_executable(mkvolume mkvolume.c)
add_executable(mkinitram mkinitram.c)

target_link_libraries(mkvolume spec_parser)
target_link_libraries(mkinitram spec_parser)

install(TARGETS mkimage mkvolume mkinitram RUNTIME DESTINATION tools)
