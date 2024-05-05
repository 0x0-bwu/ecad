#pragma once
#include "basic/ECadCommon.h"
#include <string>
namespace ecad {
class ICell;
class IPadstackDef;
class ECAD_API IComponentDefPin : public Clonable<IComponentDefPin>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponentDefPin() = default;
    virtual const std::string & GetName() const = 0;

    virtual void SetIOType(EPinIOType type) = 0;
    virtual EPinIOType GetIOType() const = 0;
    
    virtual void SetLocation(EPoint2D location) = 0;
    virtual const EPoint2D & GetLocation() const = 0;

    virtual void SetPadstackDef(CPtr<IPadstackDef> def) = 0;
    virtual CPtr<IPadstackDef> GetPadstackDef() const = 0;

    virtual void SetLayerId(ELayerId lyrId) = 0;
    virtual ELayerId GetLayerId() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponentDefPin)