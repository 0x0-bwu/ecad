#pragma once
#include <boost/geometry/index/rtree.hpp>
#include "ECadCommon.h"

namespace ecad {
namespace model {
class ELayerCutModel;
class EStackupPrismaThermalModel;
namespace utils {
class ECAD_API EStackupPrismaThermalModelQuery;
class ECAD_API EStackupPrismaThermalModelBuilder
{
public:
    explicit EStackupPrismaThermalModelBuilder(Ptr<EStackupPrismaThermalModel> model);
    virtual ~EStackupPrismaThermalModelBuilder();

    void BuildPrismaModel(EFloat scaleH2Unit, EFloat scale2Meter, size_t threads);
    void BuildPrismaInstanceTopBotNeighbors(size_t start, size_t end);

    void AddBondWiresFromLayerCutModel(CPtr<ELayerCutModel> lcm);
    static EFloat GetIntersectArea(const ETriangle2D & t1, const ETriangle2D & t2);
protected:
    Ptr<EStackupPrismaThermalModel> m_model;
    UPtr<EStackupPrismaThermalModelQuery> m_query;
};
} // namespace utils
} // namespace model
} // namespace ecad