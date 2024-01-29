#pragma once
#include "ECadCommon.h"
namespace ecad {

class ILayoutView;
namespace utils {

class ECAD_API ELayoutConnectivity
{
public:
    static void ConnectivityExtraction(Ptr<ILayoutView> layout);
    // static void ConnectivityCheck();//todo
};

}//namespace utils
}//namespace ecad