#ifndef ECAD_EMODEL_ETHERM_EGRIDTHERMALMODEL_HPP
#define ECAD_EMODEL_ETHERM_EGRIDTHERMALMODEL_HPP
#include "utilities/EMetalFractionMapping.h"
#include "generic/math/Interpolation.hpp"
#include "EThermalModel.h"
namespace ecad {

class IMaterialDef;
namespace emodel {
namespace etherm {

using namespace eutils;
using namespace generic::math;
using namespace generic::geometry;

using EGridData = OccupancyGridMap<ESimVal>;
using EGridInterpolator = OccupancyGridMap<Interpolation<ESimVal> >;

class ECAD_API EGridDataTable
{
public:
    explicit EGridDataTable(const ESize2D & size);
    virtual ~EGridDataTable();

    size_t GetSampleSize() const;
    const ESize2D & GetTableSize() const;

    bool AddSample(ESimVal key, EGridData data);
    ESimVal Query(ESimVal key, size_t x, size_t y, bool * success = nullptr) const;

private:
    void BuildInterpolater() const;
    void ResetInterpolater();

private:
    ESize2D m_size;
    std::map<ESimVal, EGridData> m_dataTable;//<key, table>
    mutable UPtr<EGridInterpolator> m_interpolator = nullptr;
};

using EGridBCModel = EGridDataTable;
using EGridPowerModel = EGridDataTable;

class ECAD_API EGridThermalLayer
{
public:
    explicit EGridThermalLayer(std::string name, SPtr<ELayerMetalFraction> metalFraction);
    virtual ~EGridThermalLayer();

    void SetThickness(FCoord thickness);
    FCoord GetThickness() const;

    void SetConductingMaterial(CPtr<IMaterialDef> material);
    CPtr<IMaterialDef> GetConductingMaterial() const;

    void SetDielectricMaterial(CPtr<IMaterialDef> material);
    CPtr<IMaterialDef> GetDielectricMaterial() const;

    bool SetPowerModel(SPtr<EGridPowerModel> pwrModel);
    CPtr<EGridPowerModel> GetPowerModel() const;

    ESimVal GetMetalFraction(size_t x, size_t y) const;

    ESize2D GetSize() const;

private:
    FCoord m_thickness = 0;//unit: m
    std::string m_name;
    std::string m_topLayer;
    std::string m_botLayer;
    CPtr<IMaterialDef> m_conductingMat = nullptr;
    CPtr<IMaterialDef> m_dielectricMat = nullptr;
    SPtr<EGridPowerModel> m_powerModel = nullptr;
    SPtr<ELayerMetalFraction> m_metalFraction = nullptr;
};

class ECAD_API EGridThermalModel : public EThermalModel
{
public:
    enum class BCType { HTC, HeatFlux, Temperature };
    explicit EGridThermalModel(const ESize2D & size, const FPoint2D & ref = FPoint2D(0, 0), FCoord elevation = 0);
    virtual ~EGridThermalModel();

    size_t TotalLayers() const;
    size_t TotalGrids() const;
    ESize3D ModelSize() const;
    const ESize2D & GridSize() const;

    bool SetScaleH(FCoord scaleH);
    FCoord GetScaleH() const;

    bool SetResolution(FCoord x, FCoord y);
    void GetResolution(FCoord & x, FCoord & y) const;
    const std::array<FCoord, 2> & GetResolution() const;

    bool AppendLayer(EGridThermalLayer layer);
    const std::vector<EGridThermalLayer> & GetLayers() const; 

    bool SetPowerModel(size_t layer, SPtr<EGridPowerModel> pwrModel);

    bool SetTopBotBCModel(SPtr<EGridBCModel> top, SPtr<EGridBCModel> bot);
    void GetTopBotBCModel(SPtr<EGridBCModel> & top, SPtr<EGridBCModel> & bot) const;

    void SetTopBotBCType(BCType top, BCType bot);
    void GetTopBotBCType(BCType & top, BCType & bot) const;

    size_t GetFlattenIndex(const ESize3D & index) const;
    ESize3D GetGridIndex(size_t index) const;

private:
    ESize2D m_size;
    FPoint2D m_ref;
    FCoord m_elevation;
    FCoord m_scaleH = 1.0;//only apply for horizontal
    std::array<FCoord, 2> m_resolution = {0, 0};//unit: m
    std::vector<EGridThermalLayer> m_stackupLayers;
    std::array<SPtr<EGridBCModel>, 2> m_bcTopBot = {nullptr, nullptr};
    std::array<BCType, 2> m_bcTypeTopBot = {BCType::HTC, BCType::HTC};
};

ECAD_ALWAYS_INLINE size_t EGridThermalModel::GetFlattenIndex(const ESize3D & index) const
{
    return m_size.x * m_size.y * index.z + m_size.y * index.x + index.y;
}

ECAD_ALWAYS_INLINE ESize3D EGridThermalModel::GetGridIndex(size_t index) const
{
    ESize3D gridIndex;
    size_t tmp = m_size.x * m_size.y;
    gridIndex.z = index / tmp;
    tmp = index % tmp;
    gridIndex.y = tmp % m_size.y;
    gridIndex.x = tmp / m_size.y;
    return gridIndex;
}

}//namespace etherm
}//namesapce emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EGridThermalModel.cpp"
#endif

#endif//ECAD_EMODEL_ETHERM_EGRIDTHERMALMODEL_HPP