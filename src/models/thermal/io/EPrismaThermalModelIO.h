#pragma once
#include "models/thermal/EPrismaThermalModel.h"

namespace ecad::emodel::etherm::io {

ECAD_API bool GenerateVTKFile(std::string_view filename, const EPrismaThermalModel & model, const std::vector<ESimVal> * temperature = nullptr, std::string * err = nullptr);

} // namespace ecad::emodel::etherm::io