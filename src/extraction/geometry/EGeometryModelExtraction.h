#pragma once
#include "basic/ECadSettings.h"
namespace ecad {
class IModel;
class ILayoutView;
namespace extraction {

class ECAD_API EGeometryModelExtraction
{
public:
    static UPtr<IModel> GenerateLayerCutModel(Ptr<ILayoutView> layout, const ELayerCutModelExtractionSettings & settings);
};
}//namespace extraction
}//namespace ecad