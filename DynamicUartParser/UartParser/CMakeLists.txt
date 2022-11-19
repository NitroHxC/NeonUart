set(LIB_NAME UartParser)
file(GLOB SOURCES "src/*.c")
add_library (${LIB_NAME} STATIC ${SOURCES})
#target_link_libraries(${LIB_NAME} freertos_kernel QueueComm Common)
target_include_directories (${LIB_NAME} PUBLIC inc)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
