#ifndef ECAD_HEADER_ONLY
#include "models/thermal/io/EGridThermalModelIO.h"
#endif

#include "generic/tools/StringHelper.hpp"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"

namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

using namespace generic::str;
using namespace generic::format;
using namespace generic::filesystem;


namespace detail {

#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_INLINE bool GenerateImageProfile(const std::string & filename, const EGridData & data, double min, double max)
{
    if(min > max)
        std::swap(min, max);
    auto range = max - min;
    auto rgbaFunc = [&min, &range](auto d) {
        int r, g, b, a = 255;
        generic::color::RGBFromScalar((d - min) / range, r, g, b);
        return std::make_tuple(r, g, b, a);
    };

    return data.WriteImgProfile(filename, rgbaFunc);    
}

#endif//BOOST_GIL_IO_PNG_SUPPORT

}//namespace detail
}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad