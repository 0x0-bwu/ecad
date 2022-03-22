#ifndef ECAD_EUTILS_ELAYOUTMERGEUTILITY_H
#define ECAD_EUTILS_ELAYOUTMERGEUTILITY_H
#include "ECadCommon.h"
namespace ecad {

class ILayoutView;
class ETransform2D;
namespace eutils {
class ECAD_API ELayoutMergeUtility
{
public:
    static void Merge(Ptr<ILayoutView> layout, CPtr<ILayoutView> other, const ETransform2D & transform);
};

}//namespace eutils
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utilities/ELayoutMergeUtility.cpp"
#endif

#endif//ECAD_EUTILS_ELAYOUTMERGEUTILITY_H
