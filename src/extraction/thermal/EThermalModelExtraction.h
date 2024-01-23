#pragma once
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {
class IModel;
class ILayoutView;
namespace extraction {
class ECAD_API EThermalModelExtraction
{
public:
    static UPtr<IModel> GenerateThermalModel(Ptr<ILayoutView> layout, const EThermalModelExtractionSettings & settings);
    static UPtr<IModel> GenerateGridThermalModel(Ptr<ILayoutView> layout, const EGridThermalModelExtractionSettings & settings);
    static UPtr<IModel> GeneratePrismaThermalModel(Ptr<ILayoutView> layout, const EPrismaThermalModelExtractionSettings & settings);    
};
}//namespace extraction
}//namespace ecad