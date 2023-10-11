
set(BINARY_NAME unit_test.exe)
add_executable(${BINARY_NAME} UnitTest.cpp)
target_include_directories(${BINARY_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(${BINARY_NAME} PRIVATE Ecad)