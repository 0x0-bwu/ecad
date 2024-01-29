#include "EThermalModel.h"

namespace ecad {
namespace model {

ECAD_INLINE EGridPowerModel::EGridPowerModel(const ESize2D & size)
{
    m_table.reset(new EGridDataTable(size));
}

ECAD_INLINE EGridPowerModel::EGridPowerModel(UPtr<EGridDataTable> table)
{
    m_table = std::move(table);
}

ECAD_INLINE EFloat EGridPowerModel::Query(EFloat key, size_t x, size_t y, bool * success) const
{
    return m_table->Query(key, x, y, success);
}

ECAD_INLINE std::pair<EFloat, EFloat> EGridPowerModel::GetRange() const
{
    return m_table->GetRange();
}

ECAD_INLINE bool EGridPowerModel::NeedInterpolation() const
{
    return m_table->NeedInterpolation();
}

ECAD_INLINE EBlockPowerModel::EBlockPowerModel(ESize2D ll, ESize2D ur, EFloat totalP)
 : ll(ll), ur(ur), totalPower(totalP)
{
}

ECAD_INLINE EFloat EBlockPowerModel::Query(EFloat key, size_t x, size_t y, bool * success) const
{
    bool in{ll.x <= x && x <= ur.x && ll.y <= y && y <= ur.y};
    if (success) *success = in;
    return in ? totalPower / Size() : 0;
}

ECAD_INLINE std::pair<EFloat, EFloat> EBlockPowerModel::GetRange() const
{
    return {totalPower / Size(), totalPower / Size()};
}

ECAD_INLINE bool EBlockPowerModel::NeedInterpolation() const
{
    return false;
}

ECAD_INLINE size_t EBlockPowerModel::Size() const
{
    return (ur.y - ll.y + 1) * (ur.x - ll.x + 1);
}

ECAD_INLINE void EThermalModel::SetUniformBC(EOrientation orient, EThermalBondaryCondition bc)
{
    m_uniformBC.emplace(orient, std::move(bc));
}

ECAD_INLINE CPtr<EThermalBondaryCondition> EThermalModel::GetUniformBC(EOrientation orient) const
{
    auto iter = m_uniformBC.find(orient);
    if (iter == m_uniformBC.cend()) return nullptr;
    if (not iter->second.isValid()) return nullptr;
    return &iter->second;
}

}//namespace model
}//namespace ecad
