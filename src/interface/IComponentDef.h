#pragma once
#include "basic/ECadCommon.h"
namespace ecad {
class ICell;
class IDatabase;
class IPadstackDef;
class IComponentDefPin;
class ECAD_API IComponentDef : public Clonable<IComponentDef>, public Printable
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IComponentDef() = default;
    virtual const std::string & GetName() const = 0;

    virtual void SetDatabase(CPtr<IDatabase> database) = 0;
    virtual CPtr<IDatabase> GetDatabase() const = 0;

    virtual void SetComponentType(EComponentType type) = 0;
    virtual EComponentType GetComponentType() const = 0;

    virtual void SetBondingBox(const EBox2D & bbox) = 0;
    virtual const EBox2D & GetBondingBox() const = 0;

    virtual void SetMaterial(const std::string & name) = 0;
    virtual const std::string & GetMaterial() const = 0;

    virtual void SetHeight(EFloat height) = 0;
    virtual EFloat GetHeight() const = 0;

    virtual void SetSolderBallBumpHeight(EFloat height) = 0;
    virtual EFloat GetSolderBallBumpHeight() const = 0;

    virtual void SetSolderFillingMaterial(const std::string & name) = 0;
    virtual const std::string & GetSolderFillingMaterial() const = 0;

    virtual Ptr<IComponentDefPin> CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef = nullptr, ELayerId lyr = noLayer) = 0;
    virtual Ptr<IComponentDefPin> FindPinByName(const std::string & name) const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IComponentDef)