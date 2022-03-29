#ifndef ECAD_EMODEL_ETHERM_ECHIPTHERMALMODEL_HPP
#define ECAD_EMODEL_ETHERM_ECHIPTHERMALMODEL_HPP
#include "EGridThermalModel.h"
#include <map>
#include <set>
namespace ecad {
namespace emodel {
namespace etherm {

struct ECTMv1Layer
{
    virtual ~ECTMv1Layer() = default;
    std::string name;
    EValue thickness = 0;//unit: um
};

struct ECTMv1MetalLayer : public ECTMv1Layer
{
    virtual ~ECTMv1MetalLayer() = default;
    EValue elevation = 0;//unit: um
};

struct ECTMv1ViaLayer : public ECTMv1MetalLayer
{
    std::string topLayer;
    std::string botLayer;
};

struct ECTMv1Header
{
    bool encrypted = false;
    std::string head = "Version 1.0 2022 R2";
    EValue resolution = 10.0;//unit um
    EValue scale = 1.0;
    FBox2D size = FBox2D(0.0, 0.0, 0.0, 0.0);//unit um
    FBox2D origin = FBox2D(0.0, 0.0, 0.0, 0.0);//unit um
    ESize2D tiles = ESize2D(0, 0);
    std::vector<EValue> temperatures = { 25.0, 50.0, 75.0, 100.0, 125.0 };
    std::vector<ECTMv1Layer> layers;
    std::vector<std::string> techLayers;
    std::vector<ECTMv1ViaLayer> viaLayers;
    std::vector<ECTMv1MetalLayer> metalLayers;
};

class EChipThermalModelV1 : public EThermalModel
{
public:
    ECTMv1Header header;
    UPtr<EGridPowerModel> powers = nullptr;
    std::map<std::string, UPtr<EGridData> > densities;
    virtual ~EChipThermalModelV1();
};

}//namespace etherm
}//namespace emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EChipThermalModel.cpp"
#endif

#endif//ECAD_EMODEL_ETHERM_ECHIPTHERMALMODEL_HPP