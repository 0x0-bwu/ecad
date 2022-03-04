#ifndef ECAD_EUIT_ELAYOUTMERGEUTILITY_H
#define ECAD_EUIT_ELAYOUTMERGEUTILITY_H
#include "ECadCommon.h"
namespace ecad {

class ILayoutView;
class ETransform2D;
namespace euti {
class ECAD_API ELayoutMergeUtility
{
public:
    static void Merge(Ptr<ILayoutView> layout, CPtr<ILayoutView> other, const ETransform2D & transform);
};

}//namespace euti
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utilities/ELayoutMergeUtility.cpp"
#endif

#endif//ECAD_EUIT_ELAYOUTMERGEUTILITY_H
