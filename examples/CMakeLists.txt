
add_executable(vlsi.exe vlsi.cpp)
target_include_directories(vlsi.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(vlsi.exe PRIVATE Ecad Threads::Threads dl)

add_executable(WolfSpeed_SetupDesign.exe wolfspeed/SetupDesign.cpp)
target_include_directories(WolfSpeed_SetupDesign.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(WolfSpeed_SetupDesign.exe PRIVATE Ecad Threads::Threads dl)

add_executable(test.exe test.cpp)
target_include_directories(test.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(test.exe PRIVATE Ecad Threads::Threads dl)