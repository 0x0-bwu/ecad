#include "ELayoutModifier.h"
#include "interface/Interface.h"
namespace ecad {
namespace utils {

bool ELayoutModifier::ModifyStackupLayerThickness(Ptr<ILayoutView> layout, const std::string & name, EFloat thickness)
{
    auto layer = layout->FindLayerByName(name);
    if (nullptr == layer) return false;
    auto stackupLayer = layer->GetStackupLayerFromLayer();
    if (nullptr == stackupLayer) return false;

    stackupLayer->SetThickness(thickness);
    UpdateLayerStackupElevation(layout);
    return true;
}

void ELayoutModifier::UpdateLayerStackupElevation(Ptr<ILayoutView> layout)
{
    std::vector<Ptr<IStackupLayer> > stackupLayers;
    layout->GetStackupLayers(stackupLayers);
    if (stackupLayers.empty()) return;

    for (size_t i = 1; i < stackupLayers.size(); ++i) {
        auto upperLayer = stackupLayers.at(i - 1);
        auto currentLayer = stackupLayers.at(i);
        currentLayer->SetElevation(upperLayer->GetElevation() - upperLayer->GetThickness());
    }
}

}//namespace utils
}//namespace ecad
