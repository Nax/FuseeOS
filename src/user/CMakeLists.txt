set(FLAGS "-fomit-frame-pointer -ffreestanding -nostdlib -fno-exceptions")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

add_executable(init init.c)
target_link_libraries(init libc)
