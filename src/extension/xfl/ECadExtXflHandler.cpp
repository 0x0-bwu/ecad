#ifndef ECAD_HEADER_ONLY
#include "ECadExtXflHandler.h"
#endif

#include "generic/geometry/Utility.hpp"
#include "generic/tools/Format.hpp"
#include "EXflParser.h"
#include "ETransform.h"
#include "Interface.h"
#include "ELayerMap.h"
#include "EDataMgr.h"
namespace ecad {
namespace ext {
namespace xfl {

namespace fmt = generic::format;

ECAD_INLINE ECadExtXflHandler::ECadExtXflHandler(const std::string & xflFile)
 : m_xflFile(xflFile){}

ECAD_INLINE SPtr<IDatabase> ECadExtXflHandler::CreateDatabase(const std::string & name, Ptr<std::string> err)
{
    EXflDB db;
    EXflReader reader(db);
    if(!reader(m_xflFile)) return nullptr;

    auto & eMgr = EDataMgr::Instance();
    if(eMgr.OpenDatabase(name)){
        if(err) *err = fmt::Format2String("Error: database %1% is already exist.", name);
        return nullptr;
    }

    //reset temporary data
    Reset();
    m_database = eMgr.CreateDatabase(name);
    if(nullptr == m_database) return nullptr;

    return m_database;
}

ECAD_INLINE void ECadExtXflHandler::Reset()
{
    m_database.reset();
}
}//namespace xfl

}//namespace ext
}//namespace ecad