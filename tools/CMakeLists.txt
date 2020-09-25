cmake_minimum_required(VERSION 3.10)
project(FuseeOS_Tools)

include_directories("${CMAKE_SOURCE_DIR}/../include")
add_executable(mkimage  mkimage.c)
add_executable(mkvolume mkvolume.c)
add_executable(mkinitram mkinitram.c)

install(TARGETS mkimage mkvolume mkinitram RUNTIME DESTINATION bin)
