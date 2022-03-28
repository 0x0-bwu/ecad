#ifndef ECAD_EMODEL_IO_ETHERMALMODELIO_HPP
#define ECAD_EMODEL_IO_ETHERMALMODELIO_HPP
#include "models/EThermalModel.h"
#include <set>
namespace ecad {
namespace emodel {
namespace io {

enum class ECtmV1LayerType { Metal = 0, Via = 1 };
struct ECtmV1Layer
{
    virtual ~ECtmV1Layer() = default;
    std::string name;
    EValue thickness = 0;//unit: um
};

struct ECtmV1MetalLayer : public ECtmV1Layer
{
    virtual ~ECtmV1MetalLayer() = default;
    EValue elevation = 0;//unit: um
};

struct ECtmV1ViaLayer : public ECtmV1MetalLayer
{
    std::string topLayer;
    std::string botLayer;
};

struct ECtmV1Header
{
    bool encrypted = false;
    std::string header = "Version 1.0 2022 R2";
    EValue resolution = 10.0;//unit um
    EValue scale = 1.0;
    FBox2D size = FBox2D(0.0, 0.0, 0.0, 0.0);//unit um
    FBox2D origin = FBox2D(0.0, 0.0, 0.0, 0.0);//unit um
    ESize2D tils = ESize2D(0, 0);
    std::set<EValue> temperatures = { 25.0, 50.0, 75.0, 100.0, 125.0 };
    std::vector<ECtmV1Layer> layers;
    std::vector<std::string> techLayers;
    std::vector<ECtmV1ViaLayer> viaLayers;
    std::vector<ECtmV1MetalLayer> metalLayers;
};

struct ECtmV1DB
{
    ECtmV1Header header;
    EGridPowerModel powers;
    std::map<std::string, EGridData> densities;
};

ECAD_API UPtr<EGridThermalModel> makeGridThermalModelFromCtmV1File(const std::string & filename, std::string * err = nullptr);

namespace detail {

ECAD_API std::string UntarCtmV1File(const std::string & filename, std::string * err = nullptr);//return untar folder if success

}//namespace detail

}//namespace io
}//namespace emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalModelIO.cpp"
#endif

#endif//ECAD_EMODEL_IO_ETHERMALMODELIO_HPP