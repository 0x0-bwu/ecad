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
    enum class BCType { HTC, HeatFlow, Temperature/*not work currently*/ };
    virtual ~EThermalModel() = default;

    virtual void SetUniformTopBotBCValue(EFloat top, EFloat bot);
    virtual void GetUniformTopBotBCValue(EFloat & t, EFloat & b) const;

    virtual void SetTopBotBCType(BCType top, BCType bot);
    virtual void GetTopBotBCType(BCType & top, BCType & bot) const;

protected:
    std::array<BCType, 2> m_bcTypeTopBot = {BCType::HTC, BCType::HTC};
    std::array<EFloat, 2> m_uniformBcTopBot{invalidFloat, invalidFloat};
};

using EGridData = OccupancyGridMap<EFloat>;
using Interpolator = boost::math::interpolators::pchip<std::vector<EFloat> >;
using EGridInterpolator = OccupancyGridMap<SPtr<Interpolator> >;

class ECAD_API EGridDataTable
{
public:
    explicit EGridDataTable(const ESize2D & size);
    virtual ~EGridDataTable();

    size_t GetSampleSize() const;
    const ESize2D & GetTableSize() const;

    bool AddSample(EFloat key, EGridData data);
    EFloat Query(EFloat key, size_t x, size_t y, bool * success = nullptr) const;

    std::list<EFloat> GetAllKeys() const;
    CPtr<EGridData> GetTable(EFloat key) const;
    std::pair<EFloat, EFloat> GetRange() const;

    bool NeedInterpolation() const;

private:
    void BuildInterpolater() const;
    void ResetInterpolater();

private:
    ESize2D m_size;
    std::map<EFloat, EGridData> m_dataTable;//<key, table>
    mutable UPtr<EGridInterpolator> m_interpolator = nullptr;
};

class ECAD_API EThermalPowerModel
{
public:
    virtual ~EThermalPowerModel() = default;
    virtual EFloat Query(EFloat key, size_t x, size_t y, bool * success = nullptr) const = 0;
    virtual std::pair<EFloat, EFloat> GetRange() const = 0;
    virtual bool NeedInterpolation() const = 0;
};

class ECAD_API EGridPowerModel : public EThermalPowerModel
{
public:
    explicit EGridPowerModel(const ESize2D & size);
    explicit EGridPowerModel(UPtr<EGridDataTable> table);
    virtual ~EGridPowerModel() = default;

    EFloat Query(EFloat key, size_t x, size_t y, bool * success = nullptr) const override;
    std::pair<EFloat, EFloat> GetRange() const override;
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
    EFloat totalPower;
    EBlockPowerModel(ESize2D ll, ESize2D ur, EFloat totalP);
    virtual ~EBlockPowerModel() = default;

    EFloat Query(EFloat key, size_t x, size_t y, bool * success = nullptr) const override;
    std::pair<EFloat, EFloat> GetRange() const override;
    bool NeedInterpolation() const override;
    size_t Size() const;
};

}//namespace etherm
}//namesapce emodel
}//namespace ecad