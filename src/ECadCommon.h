#ifndef ECAD_ECADCOMMON_H
#define ECAD_ECADCOMMON_H
#include "ECadConfig.h"
#include "ECadVersion.h"
#include "ECadUnits.h"
#include <cassert>
#ifdef ECAD_DEBUG_MODE
    #include <iostream>
    #define ECAD_DEBUG(ex) std::cout << ex << std::endl;
    #define ECAD_ASSERT(ex) assert(ex);
#else
    #define ECAD_DEBUG(ex)
    #define ECAD_ASSERT(ex) do{} while(0);
#endif//ECAD_DEBUG_MODE

#ifdef ECAD_EFFICIENCY_TRACK_MODE
    #include "generic/tools/Tools.hpp"
    #define ECAD_EFFICIENCY_TRACK(task)                                              \
    std::cout << "progress name: " << task << std::endl;                             \
    generic::tools::ProgressTimer ecadProgressTrackTimer;                            \
    /**/
#else
    #define ECAD_EFFICIENCY_TRACK(task)
#endif//ECAD_EFFICIENCY_TRACK

namespace ecad {
ECAD_ALWAYS_INLINE size_t DefaultThreads() { return 16; }
}//namespace 

#endif//ECAD_ECADCOMMON_H