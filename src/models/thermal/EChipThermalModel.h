#ifndef ECAD_EMODEL_ETHERM_ECHIPTHERMALMODEL_HPP
#define ECAD_EMODEL_ETHERM_ECHIPTHERMALMODEL_HPP
#include "EGridThermalModel.h"
#include <unordered_map>
#include <unordered_set>
namespace ecad {
namespace emodel {
namespace etherm {

struct ECTMv1Layer
{
    virtual ~ECTMv1Layer() = default;
    std::string name;
    EValue elevation = 0;//unit: um
    EValue thickness = 0;//unit: um
};

struct ECTMv1MetalLayer : public ECTMv1Layer
{
    virtual ~ECTMv1MetalLayer() = default;
};

struct ECTMv1ViaLayer : public ECTMv1Layer
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
    std::vector<EValue> temperatures;// = { 25.0, 50.0, 75.0, 100.0, 125.0 };
    std::vector<ECTMv1Layer> layers;
    std::vector<std::string> techLayers;
    std::vector<ECTMv1ViaLayer> viaLayers;
    std::vector<ECTMv1MetalLayer> metalLayers;
};

struct ECTMv1LayerStackup
{
    std::vector<ECTMv1Layer> layers;//top->bot
    std::vector<std::unordered_set<std::string> > names;

    friend std::ostream & operator<< (std::ostream & os, const ECTMv1LayerStackup & stackup);
};

class EChipThermalModelV1 : public EThermalModel
{
public:
    ECTMv1Header header;
    SPtr<EGridPowerModel> powers = nullptr;
    std::unordered_map<std::string, SPtr<EGridData> > densities;
    virtual ~EChipThermalModelV1();
    std::string GetLastMatelLayerInStackup() const;
    bool GetLayerHeightThickness(const std::string & name, EValue & height, EValue & thickness) const;
    CPtr<ECTMv1LayerStackup> GetLayerStackup(std::string * info = nullptr) const;

private:
    bool isMetalLayer(const std::string & name) const;
    void BuildLayerStackup(std::string * info = nullptr) const;

private:
    mutable UPtr<ECTMv1LayerStackup> m_layerStackup = nullptr;
};

}//namespace etherm
}//namespace emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EChipThermalModel.cpp"
#endif

#endif//ECAD_EMODEL_ETHERM_ECHIPTHERMALMODEL_HPP