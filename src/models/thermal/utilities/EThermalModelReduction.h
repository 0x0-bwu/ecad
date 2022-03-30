#ifndef ECAD_EMODEL_ETHERM_UTILS_ETHERMALMODELREDUCTION_HPP
#define ECAD_EMODEL_ETHERM_UTILS_ETHERMALMODELREDUCTION_HPP
#include "models/thermal/EChipThermalModel.h"
namespace ecad {
namespace emodel {
namespace etherm {
namespace utils {

enum class ReduceIndexMethod { Floor, Ceil };
enum class ReduceValueMethod { Average, Acumulation, Maximum, Minimum };
class ECAD_API EThermalModelReduction
{
public:
    virtual ~EThermalModelReduction();
};

class ECAD_API EChipThermalModelV1Reduction : public EThermalModelReduction
{
public:
    explicit EChipThermalModelV1Reduction(EChipThermalModelV1 & model);
    virtual ~EChipThermalModelV1Reduction();

    void Reduce();

private:
    EChipThermalModelV1 & m_model;
};

namespace detail {

ECAD_API ESize2D Reduce(const ESize2D & size, ReduceIndexMethod method);
ECAD_API EGridData Reduce(const EGridData & data, ReduceValueMethod method);
ECAD_API UPtr<EGridDataTable> Reduce(const EGridDataTable & data, ReduceValueMethod method);

}//namespace detail

}//namespace utils
}//namespace etherm
}//namesapce emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalModelReduction.cpp"
#endif

#endif//ECAD_EMODEL_ETHERM_UTILS_ETHERMALMODELREDUCTION_HPP