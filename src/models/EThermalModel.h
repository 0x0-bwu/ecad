#ifndef ECAD_EMODEL_ETHERMALMODEL_HPP
#define ECAD_EMODEL_ETHERMALMODEL_HPP
#include "utilities/EMetalFractionMapping.h"
#include "ECadCommon.h"
namespace ecad {
namespace emodel {

class ECAD_API EThermalModel
{
public:
    virtual ~EThermalModel();
};

class ECAD_API EGridThermalModel : public EThermalModel
{
public:
    virtual ~EGridThermalModel();

private:

};

}//namesapce emodel
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalModel.cpp"
#endif

#endif//ECAD_EMODEL_ETHERMALMODEL_HPP