#pragma once
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {

class ILayoutView;
namespace utils {
class ECAD_API ELayout2CtmUtility
{
public: 
    explicit ELayout2CtmUtility(Ptr<ILayoutView> layout);
    virtual ~ELayout2CtmUtility();
    
    void SetLayout2CtmSettings(const ELayout2CtmSettings & settings);
    bool GenerateCTMv1File(std::string * err = nullptr);

private:
    ELayout2CtmSettings m_settings;
    Ptr<ILayoutView> m_layout = nullptr;
};

}//namespace utils
}//namespace ecad