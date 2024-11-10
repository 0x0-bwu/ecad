
add_subdirectory(basic)
add_subdirectory(collection)
add_subdirectory(design)
add_subdirectory(utility)
add_subdirectory(model)
add_subdirectory(extension)
add_subdirectory(extraction)
add_subdirectory(solver)
add_subdirectory(simulation)

add_library(Ecad SHARED EDataMgr.cpp)

target_link_libraries(Ecad
    EcadBasic
    EcadCollection
    EcadDesign
    EcadUtility
    EcadModel
    EcadExtension
    EcadExtraction
    EcadSolver
    EcadSimulation
    ${MALLOC_LIB}
    ${PNG_LIBRARY} 
    Threads::Threads
    boost_serialization
    dl
)