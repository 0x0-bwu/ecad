#include "models/thermal/utilities/EPrismaThermalModelQuery.h"
#include "models/thermal/EPrismaThermalModel.h"

namespace ecad {
namespace model {
namespace utils {
ECAD_INLINE EPrismaThermalModelQuery::EPrismaThermalModelQuery(CPtr<EPrismaThermalModel> model)
 : m_model(model)
{
}

ECAD_INLINE void EPrismaThermalModelQuery::SearchPrismaInstances(const EBox2D & area, std::vector<size_t> & indices) const
{

}

ECAD_INLINE void EPrismaThermalModelQuery::SearchPrismaInstances(size_t layer, const EPoint2D & pt, std::vector<size_t> & indices) const
{

}

ECAD_INLINE size_t EPrismaThermalModelQuery::NearestLayer(EFloat height) const
{
    using namespace generic::math;
    const auto & layers = m_model->layers;
    if (GT(height, layers.front().elevation)) return 0;
    for (size_t i = 0; i < layers.size(); ++i) {
        auto top = layers.at(i).elevation;
        auto bot = top - layers.at(i).thickness;
        if (Within<LCRO>(height, bot, top)) return i;
    }
    return layers.size();
}

ECAD_INLINE void EPrismaThermalModelQuery::BuildIndexTree() const
{
}

}//namespace utils
}//namespace model
}//namespace ecad
