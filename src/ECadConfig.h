#pragma once
#define ECAD_DEBUG_MODE
// #define ECAD_EFFICIENCY_TRACK_MODE
#if defined(_WIN32) && defined(ECAD_SHARED_LIB)
#    ifdef ECAD_EXPORTS
#        define ECAD_API __declspec(dllexport)
#    else
#        define ECAD_API __declspec(dllimport)
#    endif
#else
#    define ECAD_API 
#endif
#define ECAD_INLINE
#define ECAD_ALWAYS_INLINE inline
#define ECAD_EOL GENERIC_DEFAULT_EOL
#define ECAD_SEPS GENERIC_FOLDER_SEPS