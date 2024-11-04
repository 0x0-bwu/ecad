
add_executable(vlsi.exe vlsi.cpp)
target_include_directories(vlsi.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(vlsi.exe PRIVATE Ecad)

add_executable(WolfSpeed_SetupDesign.exe wolfspeed/SetupDesign.cpp)
target_include_directories(WolfSpeed_SetupDesign.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(WolfSpeed_SetupDesign.exe PRIVATE Ecad)

add_executable(WolfSpeed_StaticThermal.exe wolfspeed/StaticThermal.cpp)
target_include_directories(WolfSpeed_StaticThermal.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(WolfSpeed_StaticThermal.exe PRIVATE Ecad)

add_executable(WolfSpeed_TransientThermal.exe wolfspeed/TransientThermal.cpp)
target_include_directories(WolfSpeed_TransientThermal.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(WolfSpeed_TransientThermal.exe PRIVATE Ecad)

add_executable(KiCAD_JetsonNano.exe kicad/JetsonNano.cpp)
target_include_directories(KiCAD_JetsonNano.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(KiCAD_JetsonNano.exe PRIVATE Ecad)

add_executable(test.exe test.cpp)
target_include_directories(test.exe PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(test.exe PRIVATE Ecad)