#pragma once
#include "basic/ECadCommon.h"
namespace ecad {
class IMaterialProp;
class ECAD_API IMaterialDef : public Clonable<IMaterialDef>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IMaterialDef() = default;

    virtual EMaterialId GetMaterialId() const = 0;
    virtual bool hasProperty(EMaterialPropId id) const = 0;
    virtual void SetProperty(EMaterialPropId id, UPtr<IMaterialProp> prop) = 0;
    virtual CPtr<IMaterialProp> GetProperty(EMaterialPropId id) const = 0;
    virtual void SetMaterialType(EMaterialType type) = 0;
    virtual EMaterialType GetMaterialType() const = 0;
    virtual const std::string & GetName() const = 0;
    virtual const EUuid & Uuid() const = 0;

};
} //namespace ecad

ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IMaterialDef)