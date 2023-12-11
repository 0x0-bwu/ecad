#include "models/thermal/io/EThermalModelIO.h"

#include "models/thermal/utilities/EThermalModelReduction.h"
#include "models/thermal/io/EChipThermalModelIO.h"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"
namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

ECAD_INLINE UPtr<EGridThermalModel> makeGridThermalModelFromCTMv1File(std::string_view filename, size_t reduceOrder, std::string * err)
{
    auto ctm = makeChipThermalModelFromCTMv1File(filename, err);
    if(nullptr == ctm) return nullptr;

    return makeGridThermalModelFromCTMv1Model(*ctm, reduceOrder, err);
}

ECAD_INLINE UPtr<EGridThermalModel> makeGridThermalModelFromCTMv1Model(const EChipThermalModelV1 & ctm, size_t reduceOrder, std::string * err)
{
    CPtr<EChipThermalModelV1> pCtm = &ctm;
    UPtr<EChipThermalModelV1> rdCtm = nullptr;
    if(reduceOrder != 0) {
        rdCtm = utils::makeReductionModel(ctm, reduceOrder);
        if(nullptr == rdCtm) {
            if(err) *err = "Error: failed to reduce model!";
            return nullptr;
        }
        pCtm = rdCtm.get();
    }
    
    auto stackup = pCtm->GetLayerStackup();
    if(nullptr == stackup) return nullptr;

    std::string devLyr = pCtm->GetLastMatelLayerInStackup();
    if(devLyr.empty()) return nullptr;

    const auto & tiles = pCtm->header.tiles;
    auto model = std::make_unique<EGridThermalModel>(tiles);

    for(size_t i = 0; i < stackup->layers.size(); ++i) {
        const auto & layer = stackup->layers.at(i);
        const auto & names = stackup->names.at(i);

        SPtr<ELayerMetalFraction> mf = nullptr;

        std::list<std::string> mName;
        for(const auto & name : names) {
            if(pCtm->densities.count(name))
                mName.push_back(name);
        }
        if(mName.size() == 1)
            mf = pCtm->densities.at(mName.front());
        else {
            mf = std::make_shared<ELayerMetalFraction>(tiles.x, tiles.y, 0);
            for(const auto & name : mName) {
                auto lm = pCtm->densities.at(name);
                for(size_t x = 0; x < tiles.x; ++x) {
                    for(size_t y = 0; y < tiles.y; ++y) {
                        (*mf)(x, y) += (*lm)(x, y);
                    }
                }
            }
            for(size_t x = 0; x < tiles.x; ++x) {
                for(size_t y = 0; y < tiles.y; ++y) {
                    if((*mf)(x, y) > 1) (*mf)(x, y) = 1;//wbtest
                }
            }
        }
        EGridThermalLayer gridLayer(layer.name, mf);
        gridLayer.SetThickness(layer.thickness * 1e-6);//um to m

        if(layer.name == devLyr)
            gridLayer.AddPowerModel(pCtm->powers);

        model->AppendLayer(std::move(gridLayer));
    }

    model->SetScaleH(pCtm->header.scale);

    EValue resolution = pCtm->header.resolution * 1e-6;//um to m
    model->SetResolution(resolution, resolution);

    return model;
}

ECAD_INLINE UPtr<EChipThermalModelV1> makeChipThermalModelV1FromGridThermalModel(const EGridThermalModel & model, bool encrypted, size_t reduceOrder, std::string * err)
{
    FCoord resX, resY;
    model.GetResolution(resX, resY);
    if(math::NE(resX, resY)) {
        if(err) *err = "Error: ctm model required same resolution in x, y direction!";
        return nullptr;
    }

    CPtr<EGridThermalModel> pModel = &model;
    UPtr<EGridThermalModel> rdModel = nullptr;
    if(reduceOrder != 0){
        rdModel = utils::makeReductionModel(model, reduceOrder);
        if(nullptr == rdModel) {
            if(err) *err = "Error: failed to reduce model!";
            return nullptr;
        }
        pModel = rdModel.get();
    }

    auto ctm = std::make_unique<EChipThermalModelV1>();
    auto & header = ctm->header;
    
    //header
    header.encrypted = encrypted;
    header.resolution = pModel->GetResolution()[0] * 1e6;//m to um
    header.scale = pModel->GetScaleH();
    
    const auto & tiles = pModel->GridSize();
    header.size = FBox2D(0.0, 0.0, tiles.x * header.resolution, tiles.y * header.resolution);
    header.origin = header.size;
    header.tiles = tiles;
    header.temperatures = { 25.0, 50.0, 75.0, 100.0, 125.0 };

    FCoord elevation = pModel->TotalThickness() * 1e6;//m to um
    for(const auto & layer : pModel->GetLayers()) {
        ECTMv1Layer ctmLayer;
        ctmLayer.name = layer.GetName();
        ctmLayer.elevation = elevation;
        ctmLayer.thickness = layer.GetThickness() * 1e6;//m to um
        elevation -= ctmLayer.thickness;

        if(layer.isMetalLayer()) {
            ECTMv1MetalLayer mLayer;
            mLayer.name = ctmLayer.name;
            mLayer.elevation = ctmLayer.elevation;
            mLayer.thickness = ctmLayer.thickness;
            header.metalLayers.emplace_back(std::move(mLayer));
        }
        else {
            ECTMv1ViaLayer vLayer;
            vLayer.name = ctmLayer.name;
            vLayer.topLayer = layer.GetTopLayer();
            vLayer.botLayer = layer.GetBotLayer();
            vLayer.elevation = ctmLayer.elevation;
            vLayer.thickness = ctmLayer.thickness;
            header.viaLayers.emplace_back(std::move(vLayer));
        }
        header.techLayers.push_back(ctmLayer.name);
        header.layers.emplace_back(std::move(ctmLayer));
    }

    //p-t table
    std::vector<EGridData> pts(header.temperatures.size(), EGridData(tiles.x, tiles.y, 0));
    
    bool success;
    for(const auto & layer : pModel->GetLayers()) {
        auto pwrModels = layer.GetPowerModels();
        if(pwrModels.empty()) continue;

        for (const auto & pwrModel : pwrModels) {
            for (size_t x = 0; x < tiles.x; ++x) {
                for (size_t y = 0; y < tiles.y; ++y) {
                    for (size_t i = 0; i < pts.size(); ++i) {
                        auto val = pwrModel->Query(header.temperatures.at(i), x, y, &success);
                        if (not success) continue;
                        pts[i](x, y) += val;
                    }
                }
            }
        }
    }

    auto powers = new EGridPowerModel(tiles);
    for(size_t i = 0; i < pts.size(); ++i)
        powers->GetTable().AddSample(header.temperatures.at(i), std::move(pts[i]));
    ctm->powers.reset(powers);

    //density
    for(const auto & layer : pModel->GetLayers()) {
        auto mf = layer.GetMetalFraction();
        if(nullptr == mf) continue;
        ctm->densities.insert(std::make_pair(layer.GetName(), mf));
    }

    if(ctm->densities.size() != ctm->header.layers.size()) {
        if(err) *err = "Error: layer size mismatch with metal density data size!";
        return nullptr;
    }

    return ctm;
}

}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad