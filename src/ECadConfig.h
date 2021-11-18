#ifndef ECAD_ECADCONFIG_H
#define ECAD_ECADCONFIG_H

#define ECAD_BOOST_SERIALIZATION_SUPPORT
#define ECAD_BOOST_SERIALIZATION_INCLUDE_UUID
#define ECAD_COMPILED_LIB
#define ECAD_DEBUG_MODE
#define ECAD_EFFICIENCY_TRACK_MODE

#ifdef ECAD_COMPILED_LIB
#    undef ECAD_HEADER_ONLY
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

#define ECAD_ALWAYS_INLINE inline
#endif//ECAD_ECADCONFIG_H