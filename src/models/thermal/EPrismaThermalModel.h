#pragma once
#include "EThermalModel.h"
#include "EShape.h"
namespace ecad {

class ILayoutView;
class IMaterialDef;
namespace emodel::etherm {

struct ECAD_API ECompactLayout
{
    std::vector<ENetId> nets;
    std::vector<ELayerId> layers;
    std::vector<EPolygonData> polygons;
    virtual ~ECompactLayout() = default;

    void AddShape(ENetId netId, ELayerId layerId, CPtr<EShape> shape);
    bool WriteImgView(std::string_view filename, size_t width = 512) const;
};

ECAD_API UPtr<ECompactLayout> makeCompactLayout(CPtr<ILayoutView> layout);

class ECAD_API EPrismaThermalModel : public EThermalModel
{

};

} // namespace emodel::etherm
} // namespace ecad