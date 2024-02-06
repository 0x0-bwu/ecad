#pragma once
#include "models/thermal/EGridThermalModel.h"
#include "models/thermal/EPrismaThermalModel.h"
#include "models/thermal/EStackupPrismaThermalModel.h"
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
struct EThermalModelTraits<EPrismaThermalModel>
{
    static size_t Size(const EPrismaThermalModel & model) { return model.TotalElements(); }
    static bool NeedIteration(const EPrismaThermalModel & model) { return model.NeedIteration(); }
};

template <>
struct EThermalModelTraits<EStackupPrismaThermalModel>
{
    static size_t Size(const EStackupPrismaThermalModel & model) { return model.TotalElements(); }
    static bool NeedIteration(const EStackupPrismaThermalModel & model) { return model.NeedIteration(); }
};
}//namesapce ecad::model::traits
