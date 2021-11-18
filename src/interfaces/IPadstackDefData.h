#ifndef ECAD_IPADSTACKDEFDATA_H
#define ECAD_IPADSTACKDEFDATA_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include "EShape.h"
#include "Protocol.h"
#include <string>
namespace ecad {

class ECAD_API IPadstackDefData : public Clonable<IPadstackDefData>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPadstackDefData() = default;

    virtual std::string GetMaterial() const = 0;
    virtual void SetMaterial(const std::string & material) = 0;

    virtual void SetLayers(const std::vector<std::string> & layers) = 0;
    virtual bool SetPadParameters(ELayerId layerId, UPtr<EShape> shape, const EPoint2D & offset, EValue rotation) = 0;
    virtual bool GetPadParameters(ELayerId layerId, CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const = 0;
    virtual bool SetPadParameters(const std::string & layer, UPtr<EShape> shape, const EPoint2D & offset, EValue rotation) = 0;
    virtual bool GetPadParameters(const std::string & layer, CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const = 0;
    virtual void SetViaParameters(UPtr<EShape> shape, const EPoint2D & offset, EValue rotation) = 0;
    virtual void GetViaParameters(CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IPadstackDefData)
#endif//ECAD_IPADSTACKDEFDATA_H