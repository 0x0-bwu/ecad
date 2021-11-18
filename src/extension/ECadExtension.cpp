#ifndef ECAD_HEADER_ONLY
#include "ECadExtension.h"
#endif
#include "extension/dmcdom/ECadExtDmcDomHandler.h"
#include "extension/gds/ECadExtGdsHandler.h"
namespace ecad {
namespace ext {

ECAD_INLINE SPtr<IDatabase> CreateDatabaseFromDomDmc(const std::string & name, const std::string & dmc, const std::string & dom, std::string * err)
{
    dmcdom::ECadExtDmcDomHandler handler(dmc, dom, ECoordUnits());
    return handler.CreateDatabase(name, err);
}

ECAD_INLINE SPtr<IDatabase> CreateDatabaseFromGds(const std::string & name, const std::string & gds, std::string * err)
{
    gds::ECadExtGdsHandler handler(gds);
    return handler.CreateDatabase(name, err);
}

}//namespace ext
}//namespace ecad