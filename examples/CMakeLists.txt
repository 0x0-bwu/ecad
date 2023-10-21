
add_executable(thermal.exe thermal.cpp)
target_include_directories(thermal.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(thermal.exe PRIVATE Ecad dl)

add_executable(setup_design.exe setup_design.cpp)
target_include_directories(setup_design.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(setup_design.exe PRIVATE Ecad dl)