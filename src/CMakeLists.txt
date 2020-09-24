include_directories("${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/src")

add_subdirectory(libc)
add_subdirectory(boot)
add_subdirectory(kernel)
