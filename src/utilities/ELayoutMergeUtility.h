#pragma once
#include "ECadCommon.h"
namespace ecad {

class ILayerMap;
class ILayoutView;
class ETransform2D;
namespace utils {
class ECAD_API ELayoutMergeUtility
{
public:
    static void Merge(Ptr<ILayoutView> layout, const std::string & hierName, CPtr<ILayoutView> other, CPtr<ILayerMap> layermap, const ETransform2D & transform);
};

}//namespace utils
}//namespace ecad