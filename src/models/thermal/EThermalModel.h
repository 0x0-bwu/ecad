#pragma once
#include "ECadCommon.h"
#include "utilities/EMetalFractionMapping.h"

namespace boost::math::interpolators {
template <class RandomAccessContainer> class pchip;
} // namespace boost::math::interpolators

namespace generic::geometry {
template <typename T> class OccupancyGridMap;
} //namespace generic::geometry

namespace ecad {
namespace emodel {
namespace etherm {

using namespace generic::geometry;

class ECAD_API EThermalModel
{
public:
    virtual ~EThermalModel() = default;
};

using EGridData = OccupancyGridMap<ESimVal>;
using Interpolator = boost::math::interpolators::pchip<std::vector<ESimVal> >;
using EGridInterpolator = OccupancyGridMap<SPtr<Interpolator> >;

class ECAD_API EGridDataTable
{
public:
    explicit EGridDataTable(const ESize2D & size);
    virtual ~EGridDataTable();

    size_t GetSampleSize() const;
    const ESize2D & GetTableSize() const;

    bool AddSample(ESimVal key, EGridData data);
    ESimVal Query(ESimVal key, size_t x, size_t y, bool * success = nullptr) const;

    std::list<ESimVal> GetAllKeys() const;
    CPtr<EGridData> GetTable(ESimVal key) const;
    std::pair<ESimVal, ESimVal> GetRange() const;

    bool NeedInterpolation() const;

private:
    void BuildInterpolater() const;
    void ResetInterpolater();

private:
    ESize2D m_size;
    std::map<ESimVal, EGridData> m_dataTable;//<key, table>
    mutable UPtr<EGridInterpolator> m_interpolator = nullptr;
};

class ECAD_API EThermalPowerModel
{
public:
    virtual ~EThermalPowerModel() = default;
    virtual ESimVal Query(ESimVal key, size_t x, size_t y, bool * success = nullptr) const = 0;
    virtual std::pair<ESimVal, ESimVal> GetRange() const = 0;
    virtual bool NeedInterpolation() const = 0;
};

class ECAD_API EGridPowerModel : public EThermalPowerModel
{
public:
    explicit EGridPowerModel(const ESize2D & size);
    explicit EGridPowerModel(UPtr<EGridDataTable> table);
    virtual ~EGridPowerModel() = default;

    ESimVal Query(ESimVal key, size_t x, size_t y, bool * success = nullptr) const override;
    std::pair<ESimVal, ESimVal> GetRange() const override;
    bool NeedInterpolation() const override;

    EGridDataTable & GetTable() { return *m_table;}
    const EGridDataTable & GetTable() const { return *m_table; }

private:
    std::unique_ptr<EGridDataTable> m_table;
};

class ECAD_API EBlockPowerModel : public EThermalPowerModel
{
public:
    ESize2D ll, ur;
    ESimVal totalPower;
    EBlockPowerModel(ESize2D ll, ESize2D ur, ESimVal totalP);
    virtual ~EBlockPowerModel() = default;

    ESimVal Query(ESimVal key, size_t x, size_t y, bool * success = nullptr) const override;
    std::pair<ESimVal, ESimVal> GetRange() const override;
    bool NeedInterpolation() const override;
    size_t Size() const;
};

}//namespace etherm
}//namesapce emodel
}//namespace ecad