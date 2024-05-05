#pragma once
#include "model/thermal/EChipThermalModel.h"

namespace ecad::model::io {

ECAD_API bool GenerateTxtProfile(const EGridThermalModel & model, std::string_view filename, std::string * err = nullptr);

ECAD_API bool GenerateImageProfiles(const EGridThermalModel & model, std::string_view dirName, std::string * err = nullptr);

namespace detail {
ECAD_API bool GenerateImageProfile(std::string_view filename, const EGridData & data, double min, double max);
}
}//namespace ecad::model::io