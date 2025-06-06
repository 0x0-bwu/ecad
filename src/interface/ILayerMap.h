#pragma once
#include "basic/ECadCommon.h"
namespace ecad {
class IDatabase;
class ECAD_API ILayerMap : public Clonable<ILayerMap>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ILayerMap() = default;

    virtual void SetDatabase(CPtr<IDatabase> database) = 0;
    virtual CPtr<IDatabase> GetDatabase() const = 0;
    virtual void SetName(std::string name) = 0;
    virtual const std::string & GetName() const = 0;
    virtual void SetMapping(ELayerId from, ELayerId to) = 0;
    virtual void MappingLeft(CPtr<ILayerMap> layerMap) = 0;
    virtual void MappingRight(CPtr<ILayerMap> layerMap) = 0;
    virtual ELayerId GetMappingForward(ELayerId from) const = 0;
    virtual ELayerId GetMappingBackward(ELayerId to) const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ILayerMap)