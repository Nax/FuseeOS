include_directories("${CMAKE_SOURCE_DIR}/include/fusee" "${CMAKE_SOURCE_DIR}/src")

add_subdirectory(libc)
add_subdirectory(boot)
add_subdirectory(kernel)
