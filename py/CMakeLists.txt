
## Link boost_serialization static
add_library(boost_python STATIC IMPORTED)
if(EXISTS ${BOOST_LIBRARY_PATH}/libboost_python310.a)
    set_target_properties(boost_python PROPERTIES IMPORTED_LOCATION ${BOOST_LIBRARY_PATH}/libboost_python310.a)
else()
    message(FATAL_ERROR "The IMPORTED_LOCATION of boost_python does not exist!")
endif()

add_library(PyEcad SHARED PyEcad.cpp)
include_directories(/usr/include/python3.10 ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(PyEcad boost_python Ecad)