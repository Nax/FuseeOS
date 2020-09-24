include_directories("${CMAKE_SOURCE_DIR}/include")

add_subdirectory(libc)
add_subdirectory(boot)
add_subdirectory(kernel)
