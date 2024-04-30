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
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EStackupPrismaThermalModel() = default;
public:
    friend class utils::EStackupPrismaThermalModelQuery;
    friend class utils::EStackupPrismaThermalModelBuilder;
    explicit EStackupPrismaThermalModel(CPtr<ILayoutView> layout);
    virtual ~EStackupPrismaThermalModel() = default;
    void SearchElementIndices(const std::vector<FPoint3D> & monitors, std::vector<size_t> & indices) const override;
    EModelType GetModelType() const override { return EModelType::ThermalStackupPrisma; }
};

} // namespace model
} // namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::model::EStackupPrismaThermalModel)