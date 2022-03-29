#ifndef ECAD_HEADER_ONLY
#include "models/thermal/EChipThermalModel.h"
#endif

namespace ecad {
namespace emodel {
namespace etherm {

ECAD_INLINE EChipThermalModelV1::~EChipThermalModelV1()
{
}

ECAD_INLINE bool EChipThermalModelV1::GetLayerHeightThickness(const std::string & name, EValue & height, EValue & thickness) const
{
    //if metal
    for(const auto & layer : header.metalLayers) {
        if(name == layer.name) {
            height = layer.elevation;
            thickness = layer.thickness;
            return true;
        }
    }
    //if via
    for(const auto & layer : header.viaLayers) {
        if(name == layer.name) {
            EValue topH, topT, botH, botT;
            if(GetLayerHeightThickness(layer.topLayer, topH, topT) &&
                GetLayerHeightThickness(layer.botLayer, botH, botT)) {
                height = topH - topT;
                thickness = height - botH;
                return true;
            }
        }
    }
    return false;
}

ECAD_INLINE void EChipThermalModelV1::BuildLayerStackup(std::string * info)
{
    m_layerStackup.reset(new ECTMv1LayerStackup);
    //wbtest, todo
}

}//namespace etherm
}//namespace emodel
}//namespace ecad
