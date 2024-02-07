#include "EStackupPrismaThermalModel.h"
#include "models/thermal/utils/EStackupPrismaThermalModelQuery.h"
#include "models/geometry/ELayerCutModel.h"


namespace ecad::model {

ECAD_INLINE EStackupPrismaThermalModel::EStackupPrismaThermalModel(CPtr<ILayoutView> layout)
 : EPrismaThermalModel(layout)
{
}

void EStackupPrismaThermalModel::SearchElementIndices(const std::vector<FPoint3D> & monitors, std::vector<size_t> & indices) const
{
    indices.resize(monitors.size());
    utils::EStackupPrismaThermalModelQuery query(this);
    for (size_t i = 0; i < monitors.size(); ++i) {
        const auto & point = monitors.at(i);
        auto layer = query.NearestLayer(point[2]);
        std::vector<typename utils::EStackupPrismaThermalModelQuery::RtVal> results;
        EPoint2D p(point[0] / m_scaleH2Unit, point[1] / m_scaleH2Unit);
        query.SearchNearestPrismaInstances(layer, p, 1, results);
        indices[i] = results.front().second;
    }
}
} //namespace ecad::model