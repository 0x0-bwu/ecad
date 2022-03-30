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

ECAD_INLINE void EChipThermalModelV1Reduction::Reduce()
{
    if(m_model.header.tiles.x == 1 ||
        m_model.header.tiles.y == 1) return;

    m_model.header.resolution *= 2;
    m_model.header.tiles = detail::Reduce(m_model.header.tiles, ReduceIndexMethod::Ceil);

    if(m_model.powers)
        m_model.powers = std::shared_ptr<EGridPowerModel>(detail::Reduce(*m_model.powers, ReduceValueMethod::Acumulation));

    for(auto & density : m_model.densities) {
        if(density.second)
            density.second = std::make_shared<EGridData>(detail::Reduce(*density.second, ReduceValueMethod::Average));
    }
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

ECAD_INLINE UPtr<EGridDataTable> Reduce(const EGridDataTable & data, ReduceValueMethod method)
{
    auto size = data.GetTableSize();
    auto keys = data.GetAllKeys();

    auto result = std::make_unique<EGridDataTable>(Reduce(size, ReduceIndexMethod::Ceil));

    for(auto key : keys) {
        auto table = data.GetTable(key);
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
