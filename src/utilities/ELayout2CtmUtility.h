#ifndef ECAD_EUTILS_ELAYOUT2CTMUTILITY_H
#define ECAD_EUTILS_ELAYOUT2CTMUTILITY_H
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {

class ILayoutView;
namespace eutils {
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

}//namespace eutils
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utilities/ELayout2CtmUtility.cpp"
#endif

#endif//ECAD_EUTILS_ELAYOUT2CTMUTILITY_H
