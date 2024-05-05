#include "EStackupPrismThermalModel.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::model::EStackupPrismThermalModel)

#include "model/thermal/utils/EStackupPrismThermalModelQuery.h"
#include "model/geometry/ELayerCutModel.h"

namespace ecad::model {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EStackupPrismThermalModel::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EPrismThermalModel);
}

template <typename Archive>
ECAD_INLINE void EStackupPrismThermalModel::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EPrismThermalModel);
}
    
ECAD_SERIALIZATION_FUNCTIONS_IMP(EStackupPrismThermalModel)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EStackupPrismThermalModel::EStackupPrismThermalModel(CPtr<ILayoutView> layout)
 : EPrismThermalModel(layout)
{
}

void EStackupPrismThermalModel::SearchElementIndices(const std::vector<FPoint3D> & monitors, std::vector<size_t> & indices) const
{
    indices.resize(monitors.size());
    utils::EStackupPrismThermalModelQuery query(this);
    for (size_t i = 0; i < monitors.size(); ++i) {
        const auto & point = monitors.at(i);
        auto layer = query.NearestLayer(point[2]);
        std::vector<typename utils::EStackupPrismThermalModelQuery::RtVal> results;
        EPoint2D p(point[0] / m_scaleH2Unit, point[1] / m_scaleH2Unit);
        query.SearchNearestPrismInstances(layer, p, 1, results);
        indices[i] = results.front().second;
    }
}
} //namespace ecad::model