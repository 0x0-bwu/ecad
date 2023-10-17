#pragma once
#include "ECadConfig.h"
#include "ECadSerialization.h"
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
    #define ECAD_MACRO_COMBINER(a, b) a ## b
    #define ECAD_MACRO_COMBINE(a, b) ECAD_MACRO_COMBINER(a, b)
    #define ECAD_EFFICIENCY_TRACK(task)                                              \
    std::cout << "progress name: " << task << std::endl;                             \
    generic::tools::ProgressTimer ECAD_MACRO_COMBINE(__ECADTIMER__,__LINE__);                   \
    /**/
#else
    #define ECAD_EFFICIENCY_TRACK(task)
#endif//ECAD_EFFICIENCY_TRACK