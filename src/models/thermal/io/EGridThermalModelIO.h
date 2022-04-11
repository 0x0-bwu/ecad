#ifndef ECAD_EMODEL_ETHERM_IO_EGRIDTHERMALMODELIO_HPP
#define ECAD_EMODEL_ETHERM_IO_EGRIDTHERMALMODELIO_HPP
#include "models/thermal/EChipThermalModel.h"

namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_API bool GenerateImageProfiles(const EGridThermalModel & model, const std::string & dirName, std::string * err = nullptr);
#endif//BOOST_GIL_IO_PNG_SUPPORT

namespace detail {
#ifdef BOOST_GIL_IO_PNG_SUPPORT
ECAD_API bool GenerateImageProfile(const std::string & filename, const EGridData & data, double min, double max);
#endif//BOOST_GIL_IO_PNG_SUPPORT

}//namespace detail
}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EGridThermalModelIO.cpp"
#endif

#endif//ECAD_EMODEL_ETHERM_IO_EGRIDTHERMALMODELIO_HPP