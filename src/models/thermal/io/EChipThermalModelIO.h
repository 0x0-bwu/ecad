#pragma once
#include "models/thermal/io/EGridThermalModelIO.h"
#include <set>

namespace ecad {
namespace emodel {
namespace etherm {
namespace io {

ECAD_API UPtr<EChipThermalModelV1> makeChipThermalModelFromCTMv1File(std::string_view filename, std::string * err = nullptr);
ECAD_API bool GenerateCTMv1FileFromChipThermalModelV1(const EChipThermalModelV1 & model, std::string_view dirName, std::string_view filename, std::string * err = nullptr);

ECAD_API bool GenerateCTMv1ImageProfiles(const EChipThermalModelV1 & model, std::string_view dirName, std::string * err = nullptr);

namespace detail {

ECAD_API std::string UntarCTMv1File(std::string_view filename, std::string * err = nullptr);//return untar folder if success else empty string
ECAD_API bool ParseCTMv1HeaderFile(std::string_view filename, ECTMv1Header & header, std::string * err = nullptr);
ECAD_API bool ParseCTMv1PowerFile(std::string_view filename, EGridData & powers, std::string * err = nullptr);
ECAD_API bool ParseCTMv1DensityFile(std::string_view filename, const size_t size, std::vector<SPtr<EGridData> > & density, std::string * err = nullptr);

ECAD_API bool WriteCTMv1HeaderFile(std::string_view filename, const ECTMv1Header & header, std::string * err = nullptr);
ECAD_API bool WriteCTMv1PowerFile(std::string_view filename, const EGridData & powers, std::string * err = nullptr);
ECAD_API bool WriteCTMv1DensityFile(std::string_view filename, const size_t size, FCoord res, const FPoint2D & ref, const std::vector<SPtr<EGridData> > & density, std::string * err = nullptr);
ECAD_API bool GenerateCTMv1Package(std::string_view dirName, const std::string & packName, bool removeDir, std::string * err = nullptr);

}//namespace detail
}//namespace io
}//namespace etherm
}//namespace emodel
}//namespace ecad