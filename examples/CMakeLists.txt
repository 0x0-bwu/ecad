
set(BINARY_NAME thermal.exe)
add_executable(${BINARY_NAME} thermal.cpp)
target_include_directories(${BINARY_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(${BINARY_NAME} PRIVATE Ecad)