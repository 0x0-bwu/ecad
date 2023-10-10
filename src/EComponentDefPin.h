#pragma once
#include "interfaces/IComponentDefPin.h"
#include "EObject.h"
namespace ecad {

class IPadstackDef;
class ECAD_API EComponentDefPin : public EObject, public IComponentDefPin
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponentDefPin();
public:
    EComponentDefPin(const std::string & name);
    virtual ~EComponentDefPin();

    ///Copy
    EComponentDefPin(const EComponentDefPin & other);
    EComponentDefPin & operator= (const EComponentDefPin & other);

    const std::string & GetName() const;

    void SetIOType(EPinIOType type);
    EPinIOType GetIOType() const;

    void SetLocation(EPoint2D location);
    const EPoint2D & GetLocation() const;

    void SetPadstackDef(CPtr<IPadstackDef> def);
    CPtr<IPadstackDef> GetPadstackDef() const;

    void SetLayerId(ELayerId lyrId);
    ELayerId GetLayerId() const;

protected:
    ///Copy
    virtual Ptr<EComponentDefPin> CloneImp() const override { return new EComponentDefPin(*this); }

protected:
    EPinIOType m_type;
    EPoint2D m_location;
    CPtr<IPadstackDef> m_padstackDef = nullptr;
    ELayerId m_layer = ELayerId::noLayer;
};

ECAD_ALWAYS_INLINE const std::string & EComponentDefPin::GetName() const
{
    return EObject::GetName();
}

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentDefPin)

#ifdef ECAD_HEADER_ONLY
#include "EComponentDefPin.cpp"
#endif