#pragma once
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {
class IModel;
class ILayoutView;
namespace sim {
class ECAD_API EThermalNetworkExtraction
{
public:
    void SetExtractionSettings(EThermalNetworkExtractionSettings settings);
    UPtr<IModel> GeneratePrismaThermalModel(Ptr<ILayoutView> layout, EFloat minAlpha, ECoord minLen, ECoord maxLen, size_t iteration);
    UPtr<IModel> GenerateGridThermalModel(Ptr<ILayoutView> layout);
private:
    EThermalNetworkExtractionSettings m_settings;
};
}//namespace sim
}//namespace ecad