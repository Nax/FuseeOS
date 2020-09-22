set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/boot.ld")
file(GLOB_RECURSE SOURCES "*.S")
add_executable(boot ${SOURCES})
set_target_properties(boot PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

set(FLAGS -m32 -ffreestanding -nostdlib)
target_compile_options(boot PRIVATE ${FLAGS})
target_link_options(boot PRIVATE ${FLAGS} -T${LINKER_SCRIPT})
install(TARGETS boot RUNTIME DESTINATION boot)
