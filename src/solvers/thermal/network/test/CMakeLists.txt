
add_executable(test.exe test.cpp)
target_include_directories(test.exe PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(test.exe PRIVATE ${PNG_LIBRARY} dl)