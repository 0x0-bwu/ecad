#pragma once
#include "models/thermal/EChipThermalModel.h"

namespace ecad::emodel::etherm::io {

ECAD_API bool GenerateTxtProfile(const EGridThermalModel & model, const std::string & filename, std::string * err = nullptr);

#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_API bool GenerateImageProfiles(const EGridThermalModel & model, const std::string & dirName, std::string * err = nullptr);
#endif//BOOST_GIL_IO_PNG_SUPPORT

namespace detail {
#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_API bool GenerateImageProfile(const std::string & filename, const EGridData & data, double min, double max);
#endif//BOOST_GIL_IO_PNG_SUPPORT
}
}//namespace ecad::emodel::etherm::io