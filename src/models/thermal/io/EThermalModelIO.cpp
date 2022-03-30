#ifndef ECAD_HEADER_ONLY
#include "models/thermal/io/EThermalModelIO.h"
#endif

#include "models/thermal/io/EChipThermalModelIO.h"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"
namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

ECAD_INLINE UPtr<EGridThermalModel> makeGridThermalModelFromCTMv1File(const std::string & filename, std::string * err)
{
    auto ctm = ctm::makeChipThermalModelFromCTMv1File(filename, err);
    if(nullptr == ctm) return nullptr;

    auto stackup = ctm->GetLayerStackup();
    if(nullptr == stackup) return nullptr;

    std::string devLyr = ctm->GetLastMatelLayerInStackup();
    if(devLyr.empty()) return nullptr;

    const auto & tiles = ctm->header.tiles;
    auto model = std::make_unique<EGridThermalModel>(tiles);

    for(size_t i = 0; i < stackup->layers.size(); ++i) {
        const auto & layer = stackup->layers.at(i);
        const auto & names = stackup->names.at(i);

        SPtr<ELayerMetalFraction> mf = nullptr;

        std::list<std::string> mName;
        for(const auto & name : names) {
            if(ctm->densities.count(name))
                mName.push_back(name);
        }
        if(mName.size() == 1)
            mf = ctm->densities.at(mName.front());
        else {
            mf = std::make_shared<ELayerMetalFraction>(tiles.x, tiles.y, 0);
            for(const auto & name : mName) {
                auto lm = ctm->densities.at(name);
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

        if(layer.name == devLyr) {
            GENERIC_ASSERT(gridLayer.SetPowerModel(ctm->powers))
        }

        GENERIC_ASSERT(model->AppendLayer(std::move(gridLayer)));
    }

    model->SetScaleH(ctm->header.scale);

    EValue resolution = ctm->header.resolution * 1e-6;//um to m
    model->SetResolution(resolution, resolution);

    return model;
}

}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad