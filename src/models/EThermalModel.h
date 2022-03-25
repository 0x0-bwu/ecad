#ifndef ECAD_EMODEL_ETHERMALMODEL_HPP
#define ECAD_EMODEL_ETHERMALMODEL_HPP
#include "utilities/EMetalFractionMapping.h"
#include "generic/math/Interpolation.hpp"
#include "ECadCommon.h"
namespace ecad {

class IMaterialDef;
namespace emodel {

using namespace eutils;
using namespace generic::math;
using namespace generic::geometry;

using EGridDataNumType = float;
using EGridData = OccupancyGridMap<EGridDataNumType>;
using EGridInterpolator = OccupancyGridMap<Interpolation<EGridDataNumType> >;

class ECAD_API EGridDataTable
{
public:
    explicit EGridDataTable(const ESize2D & size);
    virtual ~EGridDataTable();

    size_t GetSampleSize() const;
    const ESize2D & GetTableSize() const;

    bool AddSample(EGridDataNumType key, EGridData data);
    EGridDataNumType Query(EGridDataNumType key, size_t x, size_t y, bool * success = nullptr) const;

private:
    void BuildInterpolater() const;
    void ResetInterpolater();

private:
    ESize2D m_size;
    std::map<EGridDataNumType, EGridData> m_dataTable;//<key, table>
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

class ECAD_API EThermalModel
{
public:
    virtual ~EThermalModel();
};

class ECAD_API EGridThermalModel : public EThermalModel
{
    enum class BCType { HTC, HeatFlux, Temperature };
public:
    explicit EGridThermalModel(const ESize2D & size, const FPoint2D & ref = FPoint2D(0, 0), FCoord elevation = 0);
    virtual ~EGridThermalModel();

    size_t TotalLayers() const;
    size_t TotalGrids() const;
    ESize3D ModelSize() const;
    const ESize2D & GridSize() const;

    bool SetScale(FCoord scale);
    FCoord GetSCale() const;

    bool SetResolution(FCoord x, FCoord y);
    void GetResolution(FCoord & x, FCoord & y) const;

    bool AppendLayer(EGridThermalLayer layer);
    const std::vector<EGridThermalLayer> & GetLayers() const; 

    bool SetTopBotBCModel(SPtr<EGridBCModel> top, SPtr<EGridBCModel> bot);
    void SetTopBotBCType(BCType top, BCType bot);

private:
    ESize2D m_size;
    FPoint2D m_ref;
    FCoord m_elevation;
    FCoord m_scale = 1.0;//only apply for horizontal
    std::array<FCoord, 2> m_resolution = {0, 0};//unit: m
    std::vector<EGridThermalLayer> m_stackupLayers;
    std::array<SPtr<EGridBCModel>, 2> m_bcTopBot = {nullptr, nullptr};
    std::array<BCType, 2> m_bcTypeTopBot = {BCType::HTC, BCType::HTC};
};

}//namesapce emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalModel.cpp"
#endif

#endif//ECAD_EMODEL_ETHERMALMODEL_HPP