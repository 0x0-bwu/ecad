#pragma once
#include "models/thermal/EGridThermalModel.h"
#include "models/thermal/EPrismThermalModel.h"
#include "models/thermal/EStackupPrismThermalModel.h"
namespace ecad::model::traits {

template <typename Model>
struct EThermalModelTraits
{
    static size_t Size(const Model & /*model*/) { ECAD_ASSERT(false); return 0; }
    static bool NeedIteration(const Model & /*model*/) { ECAD_ASSERT(false); return false; }
};

template <>
struct EThermalModelTraits<EGridThermalModel>
{
    static size_t Size(const EGridThermalModel & model) { return model.TotalGrids(); }
    static bool NeedIteration(const EGridThermalModel & model) { return model.NeedIteration(); }
};

template <>
struct EThermalModelTraits<EPrismThermalModel>
{
    static size_t Size(const EPrismThermalModel & model) { return model.TotalElements(); }
    static bool NeedIteration(const EPrismThermalModel & model) { return model.NeedIteration(); }
};

template <>
struct EThermalModelTraits<EStackupPrismThermalModel>
{
    static size_t Size(const EStackupPrismThermalModel & model) { return model.TotalElements(); }
    static bool NeedIteration(const EStackupPrismThermalModel & model) { return model.NeedIteration(); }
};
}//namesapce ecad::model::traits
