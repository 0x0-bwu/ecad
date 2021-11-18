#ifndef ECAD_EUTI_ELAYOUTCONNECTIVITY_H
#define ECAD_EUTI_ELAYOUTCONNECTIVITY_H

#include "ECadCommon.h"
namespace ecad {

class ILayoutView;
namespace euti {

class ECAD_API ELayoutConnectivity
{
public:
    static void ConnectivityExtraction(Ptr<ILayoutView> layout);
    // static void ConnectivityCheck();//todo
};

}//namespace euti
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utility/ELayoutConnectivity.cpp"
#endif

#endif//ECAD_EUTI_ELAYOUTCONNECTIVITY_H
