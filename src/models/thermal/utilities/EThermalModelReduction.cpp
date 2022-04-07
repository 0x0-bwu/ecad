#ifndef ECAD_HEADER_ONLY
#include "models/thermal/utilities/EThermalModelReduction.h"
#endif

namespace ecad {
namespace emodel {
namespace etherm {
namespace utils {
ECAD_INLINE EThermalModelReduction::~EThermalModelReduction()
{
}

ECAD_INLINE EChipThermalModelV1Reduction::EChipThermalModelV1Reduction(EChipThermalModelV1 & model)
 : m_model(model)
{
}

ECAD_INLINE EChipThermalModelV1Reduction::~EChipThermalModelV1Reduction()
{
}

ECAD_INLINE bool EChipThermalModelV1Reduction::Reduce()
{
    if(m_model.header.tiles.x <= 1 || m_model.header.tiles.y <= 1) return false;

    m_model.header.resolution *= 2;
    m_model.header.tiles = detail::Reduce(m_model.header.tiles, ReduceIndexMethod::Ceil);

    if(m_model.powers)
        m_model.powers = std::shared_ptr<EGridPowerModel>(detail::Reduce(m_model.powers, ReduceValueMethod::Acumulation));

    std::unordered_map<SPtr<EGridData>, SPtr<EGridData> > densityMap;
    densityMap.insert(std::make_pair(nullptr, nullptr));
    for(auto & density : m_model.densities) {
        if(!densityMap.count(density.second)) {
            auto reduced = std::make_shared<EGridData>(detail::Reduce(*density.second, ReduceValueMethod::Average));
            densityMap.insert(std::make_pair(density.second, reduced));
        }
        density.second = densityMap.at(density.second);
    }

    return true;
}

ECAD_INLINE EGridThermalModelReduction::EGridThermalModelReduction(EGridThermalModel & model)
 : m_model(model)
{
}

ECAD_INLINE EGridThermalModelReduction::~EGridThermalModelReduction()
{
}

ECAD_INLINE bool EGridThermalModelReduction::Reduce()
{
    if(m_model.m_size.x <= 1 || m_model.m_size.y <= 1) return false;

    m_model.m_size = detail::Reduce(m_model.m_size, ReduceIndexMethod::Ceil);
    m_model.m_resolution[0] *= 2; m_model.m_resolution[1] *= 2;

    std::unordered_map<SPtr<EGridPowerModel>, SPtr<EGridPowerModel> > pwrMap;
    std::unordered_map<SPtr<ELayerMetalFraction>, SPtr<ELayerMetalFraction> > mfMap { {nullptr, nullptr} };
    for(auto & layer : m_model.m_stackupLayers) {
        if(!pwrMap.count(layer.m_powerModel)) {
            auto reduced = std::shared_ptr<EGridPowerModel>(detail::Reduce(layer.m_powerModel, ReduceValueMethod::Acumulation));
            pwrMap.insert(std::make_pair(layer.m_powerModel, reduced));
        }
        layer.m_powerModel = pwrMap.at(layer.m_powerModel);

        if(!mfMap.count(layer.m_metalFraction)){
            auto reduced = std::make_shared<ELayerMetalFraction>(detail::Reduce(*layer.m_metalFraction, ReduceValueMethod::Average));
            mfMap.insert(std::make_pair(layer.m_metalFraction, reduced));
        }
        layer.m_metalFraction = mfMap.at(layer.m_metalFraction);
    }

    std::unordered_map<SPtr<EGridBCModel>, SPtr<EGridBCModel> > bcMap { {nullptr, nullptr} };
    for(size_t i = 0; i < 2; ++i) {
        auto & bc = m_model.m_bcTopBot[i];
        auto bcType = m_model.m_bcTypeTopBot[i];
        if(!bcMap.count(bc)) {
            auto method = (bcType == EGridThermalModel::BCType::HeatFlow) ?
                            ReduceValueMethod::Acumulation : ReduceValueMethod::Average;
            auto reduced = std::shared_ptr<EGridPowerModel>(detail::Reduce(bc, method));
            bcMap.insert(std::make_pair(bc, reduced));
        }
        bc = bcMap.at(bc);
    }  
    return true;
}

ECAD_INLINE UPtr<EGridThermalModel> makeReductionModel(const EGridThermalModel & model, size_t order)
{
    auto copy = model;
    while(order != 0) {
        EGridThermalModelReduction r(copy);
        if(!r.Reduce()) return nullptr;
        order--;
    }
    return std::make_unique<EGridThermalModel>(std::move(copy));
}

ECAD_INLINE UPtr<EChipThermalModelV1> makeReductionModel(const EChipThermalModelV1 & model, size_t order)
{
    auto copy = model;
    while(order != 0) {
        EChipThermalModelV1Reduction r(copy);
        if(!r.Reduce()) return nullptr;
        order--;
    }
    return std::make_unique<EChipThermalModelV1>(std::move(copy));
}

namespace detail {

ECAD_INLINE ESize2D Reduce(const ESize2D & size, ReduceIndexMethod method)
{
    ESize2D result;
    switch (method) {
        case ReduceIndexMethod::Floor : {
            result.x = static_cast<size_t>(std::floor(0.5 * size.x));
            result.y = static_cast<size_t>(std::floor(0.5 * size.y));
            break;
        }
        case ReduceIndexMethod::Ceil : {
            result.x = static_cast<size_t>(std::ceil(0.5 * size.x));
            result.y = static_cast<size_t>(std::ceil(0.5 * size.y));
            break;
        }
    }
    return result;
}

ECAD_INLINE EGridData Reduce(const EGridData & data, ReduceValueMethod method)
{
    size_t nx = data.Width(), ny = data.Height();
    ESize2D size = Reduce(ESize2D(nx, ny), ReduceIndexMethod::Ceil);
    EGridData result(size.x, size.y, 0);
    switch(method) {
        case ReduceValueMethod::Average : {
            std::vector<size_t> count(size.x * size.y, 0);
            for(size_t x = 0; x < nx; ++x) {
                for(size_t y = 0; y < ny; ++y) {
                    auto index = Reduce(ESize2D(x, y), ReduceIndexMethod::Floor);
                    result(index.x, index.y) += data(x, y);
                    count[index.x * size.y + index.y] += 1;
                }
            }
            for(size_t i = 0; i < count.size(); ++i)
                result[i] = result[i] / count[i];
            break;
        }
        case ReduceValueMethod::Acumulation : {
            for(size_t x = 0; x < nx; ++x) {
                for(size_t y = 0; y < ny; ++y) {
                    auto index = Reduce(ESize2D(x, y), ReduceIndexMethod::Floor);
                    result(index.x, index.y) += data(x, y);
                }
            }
            break;
        }
        case ReduceValueMethod::Maximum : {
            for(size_t x = 0; x < nx; ++x) {
                for(size_t y = 0; y < ny; ++y) {
                    auto index = Reduce(ESize2D(x, y), ReduceIndexMethod::Floor);
                    result(index.x, index.y) = std::max(result(index.x, index.y), data(x, y));
                }
            }
            break;
        }
        case ReduceValueMethod::Minimum : {
            for(size_t x = 0; x < nx; ++x) {
                for(size_t y = 0; y < ny; ++y) {
                    auto index = Reduce(ESize2D(x, y), ReduceIndexMethod::Floor);
                    result(index.x, index.y) = std::min(result(index.x, index.y), data(x, y));
                }
            }
            break;
        }
        default : break;
    }
    return result;
}

ECAD_INLINE UPtr<EGridDataTable> Reduce(SPtr<EGridDataTable> data, ReduceValueMethod method)
{
    if(nullptr == data) return nullptr;
    auto size = data->GetTableSize();
    auto keys = data->GetAllKeys();

    auto result = std::make_unique<EGridDataTable>(Reduce(size, ReduceIndexMethod::Ceil));

    for(auto key : keys) {
        auto table = data->GetTable(key);
        if(nullptr == table) continue;
        result->AddSample(key, Reduce(*table, method));
    }
    return result;
}
}//namespace detail
}//namespace utils
}//namespace etherm
}//namespace emodel
}//namespace ecad
