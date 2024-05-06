#pragma once
#include "EGridThermalModel.h"
#include <unordered_map>
#include <unordered_set>
namespace ecad {
namespace model {

struct ECTMv1Layer
{
    virtual ~ECTMv1Layer() = default;
    std::string name;
    EFloat elevation = 0;//unit: um
    EFloat thickness = 0;//unit: um
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
    EFloat resolution = 10.0;//unit um
    EFloat scale = 1.0;
    FBox2D size = FBox2D(0.0, 0.0, 0.0, 0.0);//unit um
    FBox2D origin = FBox2D(0.0, 0.0, 0.0, 0.0);//unit um
    ESize2D tiles = ESize2D(0, 0);
    std::vector<EFloat> temperatures;// = { 25.0, 50.0, 75.0, 100.0, 125.0 };
    std::vector<ECTMv1Layer> layers;
    std::vector<std::string> techLayers;
    std::vector<ECTMv1ViaLayer> viaLayers;
    std::vector<ECTMv1MetalLayer> metalLayers;
};

struct ECTMv1LayerStackup
{
    std::vector<ECTMv1Layer> layers;//top->bot
    std::vector<std::unordered_set<std::string> > names;
};

namespace utils {
class EChipThermalModelV1Reduction; }//namespace utils;
class ECAD_API EChipThermalModelV1 : public EThermalModel
{
    friend class utils::EChipThermalModelV1Reduction;
public:
    ECTMv1Header header;
    SPtr<EGridPowerModel> powers = nullptr;
    std::unordered_map<std::string, SPtr<EGridData> > densities;
    EChipThermalModelV1() = default;
    virtual ~EChipThermalModelV1();

    ///Copy
    EChipThermalModelV1(const EChipThermalModelV1 & other);
    EChipThermalModelV1 & operator= (const EChipThermalModelV1 & other);

    ///Move
    EChipThermalModelV1(EChipThermalModelV1 && other);
    EChipThermalModelV1 & operator= (EChipThermalModelV1 && other);

    std::string GetLastMatelLayerInStackup() const;
    bool GetLayerHeightThickness(const std::string & name, EFloat & height, EFloat & thickness) const;
    CPtr<ECTMv1LayerStackup> GetLayerStackup(std::string * info = nullptr) const;

    EModelType GetModelType() const { return EModelType::ThermalCTMv1; }

protected:
    ///Copy
    virtual Ptr<EChipThermalModelV1> CloneImp() const override { return new EChipThermalModelV1(*this); }

private:
    bool isMetalLayer(const std::string & name) const;
    void BuildLayerStackup(std::string * info = nullptr) const;

private:
    mutable UPtr<ECTMv1LayerStackup> m_layerStackup = nullptr;
};

}//namespace model
}//namespace ecad