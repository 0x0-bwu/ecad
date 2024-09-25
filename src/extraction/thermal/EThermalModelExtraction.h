#pragma once
#include "basic/ECadSettings.h"
namespace ecad {
class IModel;
class ILayoutView;
namespace extraction {
class ECAD_API EThermalModelExtraction
{
public:
    static UPtr<IModel> GenerateThermalModel(Ptr<ILayoutView> layout, const EThermalModelExtractionSettings & settings);
    static UPtr<IModel> GenerateGridThermalModel(Ptr<ILayoutView> layout, const EGridThermalModelExtractionSettings & settings);
    static UPtr<IModel> GeneratePrismThermalModel(Ptr<ILayoutView> layout, const EPrismThermalModelExtractionSettings & settings);    
    static UPtr<IModel> GenerateStackupPrismThermalModel(Ptr<ILayoutView> layout, const EPrismThermalModelExtractionSettings & settings);
};
}//namespace extraction
}//namespace ecad