#pragma once
#include "generic/geometry/Box.hpp"
#include "ECadCommon.h"
#include "EShape.h"
namespace ecad {

class IDatabase;
namespace ext {
namespace dmcdom {

struct ECAD_API EDmcData
{
    bool isVia;
    bool isHole;
    int netId;
    int layerId;
    int sigLyr1;
    int sigLyr2;
    size_t points;
    EBox2D bbox;
    std::string netName;
    std::string lyrName;
};

ECAD_API bool ParseDomLine(const std::string & line, std::vector<EPoint2D> & points, EFloat scale);
ECAD_API bool ParseDmcLine(const std::string & line, std::vector<EDmcData> & record, EFloat scale);

class ECAD_API ECadExtDmcDomHandler
{
public:
    explicit ECadExtDmcDomHandler(const std::string & dmc, const std::string & dom, ECoordUnits units);
    Ptr<IDatabase> CreateDatabase(const std::string & name, std::string * err = nullptr);

private:
    bool ParseDomFile(std::string_view filename, std::vector<EPoint2D> & points, std::string * err = nullptr);
    bool ParseDmcFile(std::string_view filename, std::vector<EDmcData> & record, std::string * err = nullptr);

private:
    std::string m_dmc, m_dom;
    ECoordUnits m_units;
};

}//namespace dmcdom
}//namespace ext
}//namespace ecad