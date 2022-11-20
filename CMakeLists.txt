set(LIB_NAME NeonUart)
file(GLOB SOURCES "*.c")
add_library (${LIB_NAME} STATIC ${SOURCES})
target_include_directories (${LIB_NAME} PUBLIC "")
