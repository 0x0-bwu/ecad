# CMake setup
cmake_minimum_required(VERSION 3.14)

# Project name
project(Ecad)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
  	set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

# Compile options
option(ENABLE_ASSERTION "Enable assertion" OFF)
option(ENABLE_EXCEPTION "Enable exception" ON)
if(NOT ENABLE_ASSERTION)
	add_compile_definitions(GENERIC_NO_ASSERTION)	
endif()
if(NOT ENABLE_EXCEPTION)
	add_compile_definitions(GENERIC_NO_EXCEPTION)
endif()

option(ENABLE_PROFILING "Enable profiling" OFF)
if(ENABLE_PROFILING)
	add_compile_definitions(ECAD_EFFICIENCY_TRACK_MODE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
	message("-- Enable GNU Profiling")
endif()

option(ENABLE_SANITIZER "Enable sanitizer" OFF)
if (ENABLE_SANITIZER)
	add_compile_options(-fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address -Wno-uninitialized)
	add_link_options(-fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address)
	message("-- Enable Sanitizer")
endif()

option(ENABLE_APPLE_ACCELERATE "Enable apple accelerate framework" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -ffast-math -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -ffast-math -Wno-deprecated-declarations")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcompare-debug-second -Wall") #hide note message
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	add_definitions(-DGENERIC_USE_BOOST_CCMATH)
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-return-std-move")
endif()

set(DYLIB_SUFFIX "so")
# MacOs
# On macOS, add the Homebrew prefix (with appropriate suffixes) to the
# respective HINTS directories (after any user-specified locations).  This
# handles Homebrew installations into non-standard locations (not /usr/local).
# We do not use CMAKE_PREFIX_PATH for this as given the search ordering of
# find_xxx(), doing so would override any user-specified HINTS locations with
# the Homebrew version if it exists.
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")

  	add_definitions(-DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED)

  	find_program(HOMEBREW_EXECUTABLE brew)
  	mark_as_advanced(FORCE HOMEBREW_EXECUTABLE)
  	if (HOMEBREW_EXECUTABLE)
    	# Detected a Homebrew install, query for its install prefix.
    	execute_process(COMMAND ${HOMEBREW_EXECUTABLE} --prefix
      	OUTPUT_VARIABLE HOMEBREW_INSTALL_PREFIX
      	OUTPUT_STRIP_TRAILING_WHITESPACE)
    	message(STATUS "Detected Homebrew with install prefix: "
      		"${HOMEBREW_INSTALL_PREFIX}, adding to CMake search paths.")
		include_directories(${HOMEBREW_INSTALL_PREFIX}/include)
		link_directories(${HOMEBREW_INSTALL_PREFIX}/lib)
    	list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_INSTALL_PREFIX}")
    	#set(CMAKE_C_COMPILER "${HOMEBREW_INSTALL_PREFIX}/Cellar/gcc/13.2.0/bin/gcc-13")
    	#set(CMAKE_CXX_COMPILER "${HOMEBREW_INSTALL_PREFIX}/Cellar/gcc/13.2.0/bin/g++-13")
    	set(DYLIB_SUFFIX "dylib")
  	endif()

	if (ENABLE_APPLE_ACCELERATE)
		add_compile_definitions(ECAD_APPLE_ACCELERATE_SUPPORT)
		message("-- Enable Apple accelerate framework")
	endif()
endif()

# Malloc
set(ECAD_MALLOC_LIB "default" CACHE STRING "malloc library")
set_property(CACHE ECAD_MALLOC_LIB PROPERTY STRINGS default;jemalloc;tcmalloc)
string(TOLOWER ${ECAD_MALLOC_LIB} ECAD_MALLOC_LIB)
if(ECAD_MALLOC_LIB STREQUAL "jemalloc")
	set(MALLOC_LIB "jemalloc")
	message("-- Enable jemalloc")
elseif(ECAD_MALLOC_LIB STREQUAL "tcmalloc")
	set(MALLOC_LIB "tcmalloc")
	message("-- Enable tcmalloc")
else()
	set(MALLOC_LIB "")
endif()

# Boost library
set(BOOST_INCLUDE_PATH $ENV{BOOST_PATH}/include)
set(BOOST_LIBRARY_PATH $ENV{BOOST_PATH}/lib)

## Ignore BOOST deprecated headers
add_definitions(-DBOOST_ALLOW_DEPRECATED_HEADERS)
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

## Link boost_serialization
#link_directories(${BOOST_LIBRARY_PATH})
add_library(boost_serialization SHARED IMPORTED)
set(BOOST_SERIALIZATION_LIB_PATH ${BOOST_LIBRARY_PATH}/libboost_serialization.${DYLIB_SUFFIX})
if(NOT EXISTS ${BOOST_SERIALIZATION_LIB_PATH})
	set(BOOST_SERIALIZATION_LIB_PATH ${BOOST_LIBRARY_PATH}/libboost_serialization-mt-x64.${DYLIB_SUFFIX})
	if(NOT EXISTS ${BOOST_SERIALIZATION_LIB_PATH})
		message(FATAL_ERROR "Can not find boost serialization library at ${BOOST_LIBRARY_PATH}!")
	endif()
endif()
set_target_properties(boost_serialization PROPERTIES IMPORTED_LOCATION ${BOOST_SERIALIZATION_LIB_PATH})

# Link Eigen
if(DEFINED ENV{EIGEN_PATH})
	include_directories($ENV{EIGEN_PATH})
else()
	find_package(Eigen3 REQUIRED)
	include_directories(${EIGEN3_INCLUDE_DIR})
endif()

# Find package
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "FOUND OMP: ${OpenMP_CXX_LIBRARY}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
	add_definitions(-DECAD_OPEN_MP_SUPPORT)
endif()

set(THIRD_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/3rdparty)
set(GENERIC_INCLUDE_PATH ${THIRD_LIBRARY_PATH}/generic/include)
set(PYBIND11_INCLUDE_PATH ${THIRD_LIBRARY_PATH}/pybind11/include)
include_directories(${PROJECT_SOURCE_DIR}/src ${GENERIC_INCLUDE_PATH} ${PNG_INCLUDE_DIR} ${BOOST_INCLUDE_PATH})

# Compile definition
add_compile_definitions(GENERIC_BOOST_GIL_IO_PNG_SUPPORT)
add_compile_definitions(ECAD_BOOST_SERIALIZATION_SUPPORT)
add_compile_definitions(ECAD_BOOST_SERIALIZATION_INCLUDE_UUID)

# Message
message(STATUS "BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "BOOST_INCLUDE_PATH: ${BOOST_INCLUDE_PATH}")
message(STATUS "BOOST_LIBRARY_PATH: ${BOOST_LIBRARY_PATH}")
message(STATUS "CMAKE_HOST_SYSTEM: ${CMAKE_HOST_SYSTEM}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: " ${CMAKE_EXE_LINKER_FLAGS})
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message(STATUS "ENABLE_ASSERTION: ${ENABLE_ASSERTION}")
message(STATUS "ENABLE_EXCEPTION: ${ENABLE_EXCEPTION}")
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

# Tracting build time
# set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
# set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

set(EXTRA_LIBS Threads::Threads dl boost_serialization ${PNG_LIBRARY} ${MALLOC_LIB})
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)
add_subdirectory(${THIRD_LIBRARY_PATH}/pybind11)
add_subdirectory(py)
