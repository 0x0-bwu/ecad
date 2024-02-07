#pragma once

#include "EPrismaThermalModel.h"
#include "EShape.h"
namespace ecad {

using namespace generic::geometry::tri;
class ILayoutView;

namespace model {
namespace utils { 
class EStackupPrismaThermalModelQuery;
class EStackupPrismaThermalModelBuilder;
}

class ECAD_API EStackupPrismaThermalModel : public EPrismaThermalModel
{
public:
    friend class utils::EStackupPrismaThermalModelQuery;
    friend class utils::EStackupPrismaThermalModelBuilder;
    explicit EStackupPrismaThermalModel(CPtr<ILayoutView> layout);
    virtual ~EStackupPrismaThermalModel() = default;
    EModelType GetModelType() const override { return EModelType::ThermalStackupPrisma; }
};

} // namespace model
} // namespace model