#pragma once
#include "model/thermal/EChipThermalModel.h"
namespace ecad {
namespace model {
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

    bool Reduce();

private:
    EChipThermalModelV1 & m_model;
};

class ECAD_API EGridThermalModelReduction : public EThermalModelReduction
{
public:
    explicit EGridThermalModelReduction(EGridThermalModel & model);
    virtual ~EGridThermalModelReduction();

    bool Reduce();

private:
    EGridThermalModel & m_model;
};

ECAD_API UPtr<EGridThermalModel> makeReductionModel(const EGridThermalModel & model, size_t order = 1);
ECAD_API UPtr<EChipThermalModelV1> makeReductionModel(const EChipThermalModelV1 & model, size_t order = 1);

namespace detail {

ECAD_API ESize2D Reduce(const ESize2D & size, ReduceIndexMethod method);
ECAD_API EGridData Reduce(const EGridData & data, ReduceValueMethod method);
ECAD_API UPtr<EGridDataTable> Reduce(const EGridDataTable & data, ReduceValueMethod method);

}//namespace detail

}//namespace utils
}//namesapce model
}//namespace ecad