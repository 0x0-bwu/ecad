#pragma once
#include "ECadCommon.h"
namespace ecad {

class ILayerMap;
class ILayoutView;
class ETransform2D;
namespace eutils {
class ECAD_API ELayoutMergeUtility
{
public:
    static void Merge(Ptr<ILayoutView> layout, CPtr<ILayoutView> other, CPtr<ILayerMap> layermap, const ETransform2D & transform);
};

}//namespace eutils
}//namespace ecad