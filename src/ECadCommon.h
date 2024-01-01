#pragma once
#include "ECadConfig.h"
#include "ECadSerialization.h"
#include "ECadVersion.h"
#include "ECadUnits.h"

#include "generic/tools/Log.hpp"
#include <cassert>

#ifdef ECAD_DEBUG_MODE
    #define ECAD_ASSERT(ex) assert(ex);
#else
    #define ECAD_ASSERT(ex) do{} while(0);
#endif//ECAD_DEBUG_MODE

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

#define ECAD_TRACE(args...) generic::log::Trace(args);
#define ECAD_DEBUG(args...) generic::log::Debug(args);
#define ECAD_INFO (args...) generic::log::Info (args);