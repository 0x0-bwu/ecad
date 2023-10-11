#pragma once
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