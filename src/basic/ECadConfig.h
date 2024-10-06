#pragma once
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

#include "generic/common/Exception.hpp"
#define ECAD_ASSERT(ex) GENERIC_ASSERT(ex);

#ifdef ECAD_EFFICIENCY_TRACK_MODE
    #include "generic/tools/Tools.hpp"
    #define ECAD_MACRO_COMBINER(a, b) a ## b
    #define ECAD_MACRO_COMBINE(a, b) ECAD_MACRO_COMBINER(a, b)
    #define ECAD_EFFICIENCY_TRACK(task)                                                                                \
    std::cout << "progress name: " << task << std::endl;                                                               \
    generic::tools::ProgressTimer ECAD_MACRO_COMBINE(__ECADTIMER__,__LINE__)(task, generic::unit::Time::Millisecond);  \
    /**/
#else
    #define ECAD_EFFICIENCY_TRACK(task)
#endif//ECAD_EFFICIENCY_TRACK
