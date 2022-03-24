#ifndef ECAD_HEADER_ONLY
#include "models/EThermalModel.h"
#endif

#include "interfaces/IMaterialDef.h"
namespace ecad {
namespace emodel {

ECAD_INLINE EThermalModel::~EThermalModel()
{
}

ECAD_INLINE EGridPowerModel::~EGridPowerModel()
{
}

ECAD_INLINE EGridThermalLayer::EGridThermalLayer(std::string name, SPtr<ELayerMetalFraction> metalFraction)
 : m_name(std::move(name)), m_metalFraction(metalFraction)
{
    GENERIC_ASSERT(m_metalFraction != nullptr)
}

ECAD_INLINE EGridThermalLayer::~EGridThermalLayer()
{
}

ECAD_INLINE void EGridThermalLayer::SetPowerModel(SPtr<EGridPowerModel> pwrModel)
{
    m_powerModel = pwrModel;
}

ECAD_INLINE ESize2D EGridThermalLayer::GetSize() const
{
    auto w = m_metalFraction.Width();
    auto h = m_metalFraction.Height();
    return ESize2D{w, h};
}

ECAD_INLINE EGridThermalModel::~EGridThermalModel()
{
}

}//namespace emodel
}//namespace ecad
