#include "ECadExtension.h"

#include "extension/dmcdom/ECadExtDmcDomHandler.h"
#include "extension/kicad/ECadExtKiCadHandler.h"
#include "extension/gds/ECadExtGdsHandler.h"
#include "extension/xfl/ECadExtXflHandler.h"
namespace ecad::ext {

ECAD_INLINE Ptr<IDatabase> CreateDatabaseFromDomDmc(const std::string & name, const std::string & dmc, const std::string & dom, std::string * err)
{
    dmcdom::ECadExtDmcDomHandler handler(dmc, dom, ECoordUnits());
    return handler.CreateDatabase(name, err);
}

ECAD_INLINE Ptr<IDatabase> CreateDatabaseFromKiCad(const std::string & name, const std::string & kicad, std::string * err)
{
    kicad::ECadExtKiCadHandler handler(kicad);
    return handler.CreateDatabase(name, err); 
}

ECAD_INLINE Ptr<IDatabase> CreateDatabaseFromGds(const std::string & name, const std::string & gds, const std::string & lyrMap, std::string * err)
{
    gds::ECadExtGdsHandler handler(gds, lyrMap);
    return handler.CreateDatabase(name, err);
}

ECAD_INLINE Ptr<IDatabase> CreateDatabaseFromXfl(const std::string & name, const std::string & xfl, std::string * err)
{
    xfl::ECadExtXflHandler handler(xfl);
    return handler.CreateDatabase(name, err);
}

}//namespace ecad::ext