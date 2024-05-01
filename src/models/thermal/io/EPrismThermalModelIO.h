#pragma once
#include "models/thermal/EPrismThermalModel.h"

namespace ecad::model::io {

ECAD_API bool GenerateVTKFile(std::string_view filename, const EPrismThermalModel & model, const std::vector<EFloat> * temperature = nullptr, std::string * err = nullptr);

} // namespace ecad::model::io