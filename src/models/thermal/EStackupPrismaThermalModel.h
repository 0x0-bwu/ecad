#pragma once

#include "EPrismaThermalModel.h"
#include "EShape.h"
namespace ecad {

using namespace generic::geometry::tri;
class ILayoutView;

namespace model {
namespace utils { class EStackupPrismaThermalModelQuery; }
class ECAD_API EStackupPrismaThermalModel : public EPrismaThermalModel
{
public:
    friend class utils::EStackupPrismaThermalModelQuery;
    explicit EStackupPrismaThermalModel(CPtr<ILayoutView> layout);
    virtual ~EStackupPrismaThermalModel() = default;

    void BuildPrismaModel(EFloat scaleH2Unit, EFloat scale2Meter) override;
    EModelType GetModelType() const override { return EModelType::ThermalStackupPrisma; }
};

} // namespace model
} // namespace model