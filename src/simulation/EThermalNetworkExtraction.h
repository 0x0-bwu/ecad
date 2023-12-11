#pragma once
#include "models/thermal/EThermalModel.h"
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {
class ILayoutView;
namespace esim {
using namespace emodel::etherm;
class ECAD_API EThermalNetworkExtraction
{
public:
    void SetExtractionSettings(EThermalNetworkExtractionSettings settings);
    UPtr<EThermalModel> GeneratePrismaThermalModel(Ptr<ILayoutView> layout);
    UPtr<EThermalModel> GenerateGridThermalModel(Ptr<ILayoutView> layout);
private:
    EThermalNetworkExtractionSettings m_settings;
};
}//namespace esim
}//namespace ecad