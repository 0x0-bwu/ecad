#pragma once

#include "EPrismThermalModel.h"
#include "EShape.h"
namespace ecad {

using namespace generic::geometry::tri;
class ILayoutView;

namespace model {
namespace utils { 
class EStackupPrismThermalModelQuery;
class EStackupPrismThermalModelBuilder;
}

class ECAD_API EStackupPrismThermalModel : public EPrismThermalModel
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EStackupPrismThermalModel() = default;
public:
    friend class utils::EStackupPrismThermalModelQuery;
    friend class utils::EStackupPrismThermalModelBuilder;
    explicit EStackupPrismThermalModel(CPtr<ILayoutView> layout);
    virtual ~EStackupPrismThermalModel() = default;
    void SearchElementIndices(const std::vector<FPoint3D> & monitors, std::vector<size_t> & indices) const override;
    EModelType GetModelType() const override { return EModelType::ThermalStackupPrism; }
};

} // namespace model
} // namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::model::EStackupPrismThermalModel)