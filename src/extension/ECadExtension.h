#pragma once
#include "ECadCommon.h"
namespace ecad {

class IDatabase;
namespace ext {

ECAD_API SPtr<IDatabase> CreateDatabaseFromDomDmc(const std::string & name, const std::string & dmc, const std::string & dom, std::string * err = nullptr);
ECAD_API SPtr<IDatabase> CreateDatabaseFromGds(const std::string & name, const std::string & gds, const std::string & lyrMap = std::string{}, std::string * err = nullptr);
ECAD_API SPtr<IDatabase> CreateDatabaseFromXfl(const std::string & name, const std::string & xfl, std::string * err = nullptr);

}//namespace ext
}//namespace ecad