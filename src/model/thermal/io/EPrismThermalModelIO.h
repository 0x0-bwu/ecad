#pragma once
#include "model/thermal/EPrismThermalModel.h"

namespace ecad::model::io {

template <typename Scalar>
ECAD_API bool GenerateVTKFile(std::string_view filename, const EPrismThermalModel & model, const std::vector<Scalar> * temperature = nullptr, std::string * err = nullptr);

} // namespace ecad::model::io