
add_subdirectory(basic)
add_subdirectory(collection)
add_subdirectory(design)
add_subdirectory(utility)
add_subdirectory(model)
add_subdirectory(extension)
add_subdirectory(extraction)
add_subdirectory(solver)
add_subdirectory(simulation)

add_library(Ecad SHARED
EDataMgr.cpp
)

target_link_libraries(Ecad
    Ebasic
    Ecollection
    Edesign
    Eutility
    Emodel
    Eextension
    Eextraction
    Esolver
    Esimulation
    Emesh 
    ${PNG_LIBRARY} 
    boost_serialization
)