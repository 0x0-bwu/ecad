#ifndef ECAD_EMODEL_ETHERMALMODEL_HPP
#define ECAD_EMODEL_ETHERMALMODEL_HPP
#include "utilities/EMetalFractionMapping.h"
#include "ECadCommon.h"
namespace ecad {

class IMaterialDef;
namespace emodel {

using namespace generic::geometry;

using eutils::ELayerMetalFraction;
using EGridPower = OccupancyGridMap<float>;

class ECAD_API EThermalModel
{
public:
    virtual ~EThermalModel();
};

class ECAD_API EGridPowerModel
{
public:
    virtual ~EGridPowerModel();

private:
    std::map<double, EGridPower> m_powerTable;//<temperature, power>
};

class ECAD_API EGridThermalLayer
{
public:
    explicit EGridThermalLayer(std::string name, SPtr<ELayerMetalFraction> metalFraction);
    virtual ~EGridThermalLayer();

    void SetScale(FCoord scale);
    FCoord GetScale() const;

    void SetThickness(FCoord thickness);
    FCoord GetThickness() const;

    void SetElevation(FCoord elevation);
    FCoord GetElevation() const;

    void SetPowerModel(SPtr<EGridPowerModel> pwrModel);
    ESize2D GetSize() const;

private:
    FCoord m_scale = 1.0;
    FCoord m_elevation = 0;//unit: m
    FCoord m_thickness = 0;//unit: m
    std::array<FCoord, 2> m_resolution = 0;//unit: m
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
    virtual ~EGridThermalModel();

private:
    std::vector<EGridThermalLayer> m_stackupLayers;
};

}//namesapce emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalModel.cpp"
#endif

#endif//ECAD_EMODEL_ETHERMALMODEL_HPP