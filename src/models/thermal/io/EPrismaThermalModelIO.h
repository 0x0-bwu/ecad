#pragma once
#include "models/thermal/EPrismaThermalModel.h"

namespace ecad::emodel::etherm::io {

ECAD_API bool GenerateVTKFile(const EPrismaThermalModel & model, std::string_view filename, std::string * err = nullptr);

} // namespace ecad::emodel::etherm::io