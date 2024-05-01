#pragma once
#include <boost/geometry/index/rtree.hpp>
#include "ECadCommon.h"

namespace ecad {
namespace model {
class ELayerCutModel;
class EStackupPrismThermalModel;
namespace utils {
class ECAD_API EStackupPrismThermalModelQuery;
class ECAD_API EStackupPrismThermalModelBuilder
{
public:
    explicit EStackupPrismThermalModelBuilder(Ptr<EStackupPrismThermalModel> model);
    virtual ~EStackupPrismThermalModelBuilder();

    void BuildPrismModel(EFloat scaleH2Unit, EFloat scale2Meter, size_t threads);
    void BuildPrismInstanceTopBotNeighbors(size_t start, size_t end);

    void AddBondWiresFromLayerCutModel(CPtr<ELayerCutModel> lcm);
    static EFloat GetIntersectArea(const ETriangle2D & t1, const ETriangle2D & t2);
protected:
    Ptr<EStackupPrismThermalModel> m_model;
    UPtr<EStackupPrismThermalModelQuery> m_query;
};
} // namespace utils
} // namespace model
} // namespace ecad