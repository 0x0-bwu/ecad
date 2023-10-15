
## Link boost_serialization static
add_library(boost_python SHARED IMPORTED)
if(EXISTS ${BOOST_LIBRARY_PATH}/libboost_python${PYVERSION}.${DYLIB_SUFFIX})
    set_target_properties(boost_python PROPERTIES IMPORTED_LOCATION ${BOOST_LIBRARY_PATH}/libboost_python${PYVERSION}.${DYLIB_SUFFIX})
else()
    message(FATAL_ERROR "The IMPORTED_LOCATION of boost_python does not exist!")
endif()

add_library(PyEcad SHARED PyEcad.cpp)
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    include_directories(/Users/bwu/opt/anaconda3/include/python3.9 ${PROJECT_SOURCE_DIR}/src)
else()
    include_directories(/usr/include/python3.10 ${PROJECT_SOURCE_DIR}/src)
endif()
target_link_libraries(PyEcad boost_python Ecad)