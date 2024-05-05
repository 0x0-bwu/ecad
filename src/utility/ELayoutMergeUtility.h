#pragma once
#include "basic/ECadCommon.h"
namespace ecad {

class ICellInst;
class ILayoutView;
class ETransform2D;
namespace utils {
class ECAD_API ELayoutMergeUtility
{
public:
    static bool Merge(Ptr<ILayoutView> layout, CPtr<ICellInst> cellInst);
};

}//namespace utils
}//namespace ecad