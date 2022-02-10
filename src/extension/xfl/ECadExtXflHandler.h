#ifndef ECAD_EXT_XFL_ECADEXTXFLHANDLER_H
#define ECAD_EXT_XFL_ECADEXTXFLHANDLER_H
#include "extension/xfl/EXflObjects.h"
#include "ECadCommon.h"
#include "Interface.h"
#include <map>
namespace ecad {
namespace ext {
namespace xfl {

class ECAD_API ECadExtXflHandler
{
public:
    explicit ECadExtXflHandler(const std::string & xflFile);
    SPtr<IDatabase> CreateDatabase(const std::string & name, Ptr<std::string> err = nullptr);
private:
    void Reset();
private:
    std::string m_xflFile;

private:
    // temporary data
    SPtr<IDatabase> m_database = nullptr;
};

}//namespace xfl
}//namespace ext
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "ECadExtXflHandler.cpp"
#endif

#endif//ECAD_EXT_XFL_ECADEXTXFLHANDLER_H