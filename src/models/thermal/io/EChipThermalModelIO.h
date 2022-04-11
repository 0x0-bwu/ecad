#ifndef ECAD_EMODEL_ETHERM_IO_ECHIPTHERMALMODELIO_HPP
#define ECAD_EMODEL_ETHERM_IO_ECHIPTHERMALMODELIO_HPP
#include "models/thermal/io/EGridThermalModelIO.h"
#include <set>

namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

ECAD_API UPtr<EChipThermalModelV1> makeChipThermalModelFromCTMv1File(const std::string & filename, std::string * err = nullptr);
ECAD_API bool GenerateCTMv1FileFromChipThermalModelV1(const EChipThermalModelV1 & model, const std::string & dirName, const std::string & filename, std::string * err = nullptr);

#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_API bool GenerateCTMv1ImageProfiles(const EChipThermalModelV1 & model, const std::string & dirName, std::string * err = nullptr);
#endif//BOOST_GIL_IO_PNG_SUPPORT


namespace detail {

ECAD_API std::string UntarCTMv1File(const std::string & filename, std::string * err = nullptr);//return untar folder if success else empty string
ECAD_API bool ParseCTMv1HeaderFile(const std::string & filename, ECTMv1Header & header, std::string * err = nullptr);
ECAD_API bool ParseCTMv1PowerFile(const std::string & filename, EGridData & powers, std::string * err = nullptr);
ECAD_API bool ParseCTMv1DensityFile(const std::string & filename, const size_t size, std::vector<SPtr<EGridData> > & density, std::string * err = nullptr);

ECAD_API bool WriteCTMv1HeaderFile(const std::string & filename, const ECTMv1Header & header, std::string * err = nullptr);
ECAD_API bool WriteCTMv1PowerFile(const std::string & filename, const EGridData & powers, std::string * err = nullptr);
ECAD_API bool WriteCTMv1DensityFile(const std::string & filename, const size_t size, FCoord res, const FPoint2D & ref, const std::vector<SPtr<EGridData> > & density, std::string * err = nullptr);
ECAD_API bool GenerateCTMv1Package(const std::string & dirName, const std::string & packName, bool removeDir, std::string * err = nullptr);

}//namespace detail
}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EChipThermalModelIO.cpp"
#endif

#endif//ECAD_EMODEL_ETHERM_IO_ECHIPTHERMALMODELIO_HPP