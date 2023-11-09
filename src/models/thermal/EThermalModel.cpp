#include "models/thermal/EThermalModel.h"

namespace ecad {
namespace emodel {
namespace etherm {

ECAD_INLINE EGridPowerModel::EGridPowerModel(const ESize2D & size)
{
    m_table.reset(new EGridDataTable(size));
}

ECAD_INLINE EGridPowerModel::EGridPowerModel(UPtr<EGridDataTable> table)
{
    m_table = std::move(table);
}

ECAD_INLINE ESimVal EGridPowerModel::Query(ESimVal key, size_t x, size_t y, bool * success) const
{
    return m_table->Query(key, x, y, success);
}

ECAD_INLINE std::pair<ESimVal, ESimVal> EGridPowerModel::GetRange() const
{
    return m_table->GetRange();
}

ECAD_INLINE bool EGridPowerModel::NeedInterpolation() const
{
    return m_table->NeedInterpolation();
}

ECAD_INLINE EBlockPowerModel::EBlockPowerModel(ESize2D ll, ESize2D ur, ESimVal totalP)
 : ll(ll), ur(ur), totalPower(totalP)
{
}

ECAD_INLINE ESimVal EBlockPowerModel::Query(ESimVal key, size_t x, size_t y, bool * success) const
{
    bool in{ll.x <= x && x <= ur.x && ll.y <= y && y <= ur.y};
    if (success) *success = in;
    return in ? totalPower / Size() : 0;
}

ECAD_INLINE std::pair<ESimVal, ESimVal> EBlockPowerModel::GetRange() const
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

}//namespace etherm
}//namespace emodel
}//namespace ecad
