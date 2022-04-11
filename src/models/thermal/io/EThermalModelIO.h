#ifndef ECAD_EMODEL_ETHERM_IO_ETHERMALMODELIO_HPP
#define ECAD_EMODEL_ETHERM_IO_ETHERMALMODELIO_HPP
#include "models/thermal/EChipThermalModel.h"
#include <set>
namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

ECAD_API UPtr<EGridThermalModel> makeGridThermalModelFromCTMv1File(const std::string & filename, size_t reduceOrder = 0, std::string * err = nullptr);
ECAD_API UPtr<EGridThermalModel> makeGridThermalModelFromCTMv1Model(const EChipThermalModelV1 & ctm, size_t reduceOrder = 0, std::string * err = nullptr);
ECAD_API UPtr<EChipThermalModelV1> makeChipThermalModelV1FromGridThermalModel(const EGridThermalModel & model, bool encrypted, size_t reduceOrder = 0, std::string * err = nullptr);

}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalModelIO.cpp"
#endif

#endif//ECAD_EMODEL_ETHERM_IO_ETHERMALMODELIO_HPP