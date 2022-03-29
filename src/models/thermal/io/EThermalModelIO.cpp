#ifndef ECAD_HEADER_ONLY
#include "models/thermal/io/EThermalModelIO.h"
#endif

#include "models/thermal/io/EChipThermalModelIO.h"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"
namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

ECAD_INLINE UPtr<EGridThermalModel> makeGridThermalModelFromCTMv1File(const std::string & filename, std::string * err)
{
    auto ctm = ctm::makeChipThermalModelFromCTMv1File(filename, err);
    if(nullptr == ctm) return nullptr;

    auto dir = generic::filesystem::DirName(filename);
    ctm::GenerateCTMv1ImageProfiles(*ctm, dir + "/export");

    return nullptr;//wbtest
}

}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad