find_program(PYTHON_EXECUTABLE python)
message(STATUS "PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}")

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
message(STATUS "PYTHON_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")

set(PY_VERSION ${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR})
message(STATUS "PYTHON_VERSION: ${PY_VERSION}")

set(BOOST_PYTHON_LIB_PATH "")
## Link boost_python
add_library(boost_python SHARED IMPORTED)
set(BOOST_PYTHON_LIB_PATH  ${BOOST_LIBRARY_PATH}/libboost_python${PY_VERSION}.${DYLIB_SUFFIX})
if(NOT EXISTS ${BOOST_PYTHON_LIB_PATH})
    set(BOOST_PYTHON_LIB_PATH  ${BOOST_LIBRARY_PATH}/libboost_python${PY_VERSION}-mt-x64.${DYLIB_SUFFIX})
    if(NOT EXISTS ${BOOST_PYTHON_LIB_PATH})
        message(FATAL_ERROR "Can not find boost python library ${BOOST_PYTHON_LIB_PATH}!")
    endif()
endif()
set_target_properties(boost_python PROPERTIES IMPORTED_LOCATION ${BOOST_PYTHON_LIB_PATH})

set(CMAKE_SHARED_MODULE_PREFIX "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-uninitialized")

add_library(PyEcad MODULE PyEcad.cpp)
target_link_libraries(PyEcad Ecad boost_python ${Python3_LIBRARIES})
target_include_directories(PyEcad PRIVATE ${Python3_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src)
