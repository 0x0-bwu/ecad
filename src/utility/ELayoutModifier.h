#pragma once
#include "basic/ECadCommon.h"
namespace ecad {

class ILayer;
class IBondwire;
class IComponent;
class ILayoutView;
class IStackupLayer;
namespace utils {
class ECAD_API ELayoutModifier
{
public:
    static bool ModifyStackupLayerThickness(Ptr<ILayoutView> layout, const std::string & lyrName, EFloat thickness);

private:
    static void UpdateLayerStackupElevation(Ptr<ILayoutView> layout);
};

}//namespace utils
}//namespace ecad