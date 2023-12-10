#pragma once
#include "models/thermal/EChipThermalModel.h"

namespace ecad::emodel::etherm::io {

ECAD_API bool GenerateTxtProfile(const EGridThermalModel & model, const std::string & filename, std::string * err = nullptr);

ECAD_API bool GenerateImageProfiles(const EGridThermalModel & model, const std::string & dirName, std::string * err = nullptr);

namespace detail {
ECAD_API bool GenerateImageProfile(const std::string & filename, const EGridData & data, double min, double max);
}
}//namespace ecad::emodel::etherm::io