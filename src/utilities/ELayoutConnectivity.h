#ifndef ECAD_EUTILS_ELAYOUTCONNECTIVITY_H
#define ECAD_EUTILS_ELAYOUTCONNECTIVITY_H

#include "ECadCommon.h"
namespace ecad {

class ILayoutView;
namespace eutils {

class ECAD_API ELayoutConnectivity
{
public:
    static void ConnectivityExtraction(Ptr<ILayoutView> layout);
    // static void ConnectivityCheck();//todo
};

}//namespace eutils
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utilities/ELayoutConnectivity.cpp"
#endif

#endif//ECAD_EUTILS_ELAYOUTCONNECTIVITY_H
