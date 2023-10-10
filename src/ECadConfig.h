#pragma once
#define ECAD_BOOST_SERIALIZATION_SUPPORT
#define ECAD_BOOST_SERIALIZATION_INCLUDE_UUID
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
#endif

#define ECAD_COMPILED_LIB
#define ECAD_DEBUG_MODE
#define ECAD_EFFICIENCY_TRACK_MODE

#ifdef ECAD_COMPILED_LIB
#    undef ECAD_HEADER_ONLY
#    define ECAD_LIB_NAME PyEcad
#    define ECAD_BOOST_PYTHON_SUPPORT
#    if defined(_WIN32) && defined(ECAD_SHARED_LIB)
#        ifdef ECAD_EXPORTS
#            define ECAD_API __declspec(dllexport)
#        else
#            define ECAD_API __declspec(dllimport)
#        endif
#    else
#        define ECAD_API
#    endif
#    define ECAD_INLINE
#else
#    define ECAD_API
#    define ECAD_HEADER_ONLY
#    define ECAD_INLINE inline
#endif//ECAD_COMPILED_LIB

#define ECAD_EOL GENERIC_DEFAULT_EOL
#define ECAD_ALWAYS_INLINE inline