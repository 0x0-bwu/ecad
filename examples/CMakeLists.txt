
add_executable(vlsi.exe vlsi.cpp)
target_include_directories(vlsi.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(vlsi.exe PRIVATE Ecad Threads::Threads dl)

add_executable(sic.exe sic.cpp)
target_include_directories(sic.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(sic.exe PRIVATE Ecad Threads::Threads dl)