include_directories(
    "${CMAKE_SOURCE_DIR}/include/fusee"
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/include/fusee/c++>
    "${CMAKE_SOURCE_DIR}/src"
)

set(FLAGS_COMMON        -fomit-frame-pointer)
set(FLAGS_FREESTANDING  ${FLAGS_COMMON} -ffreestanding -nostdlib -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics> $<$<COMPILE_LANGUAGE:CXX>:-std=c++14>)
set(FLAGS_BOOT          ${FLAGS_FREESTANDING} -mno-sse -mno-sse2)
set(FLAGS_KERNEL        ${FLAGS_FREESTANDING} -mcmodel=kernel -mno-red-zone)

add_subdirectory(libc)
add_subdirectory(libboot)
add_subdirectory(boot)
add_subdirectory(kernel)
