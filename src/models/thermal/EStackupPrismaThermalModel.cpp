#include "EStackupPrismaThermalModel.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::model::EStackupPrismaThermalModel)

#include "models/thermal/utils/EStackupPrismaThermalModelQuery.h"
#include "models/geometry/ELayerCutModel.h"

namespace ecad::model {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EStackupPrismaThermalModel::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EPrismaThermalModel);
}

template <typename Archive>
ECAD_INLINE void EStackupPrismaThermalModel::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EPrismaThermalModel);
}
    
ECAD_SERIALIZATION_FUNCTIONS_IMP(EStackupPrismaThermalModel)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

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