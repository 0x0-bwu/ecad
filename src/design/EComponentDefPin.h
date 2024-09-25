#pragma once
#include "interface/IComponentDefPin.h"
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

    const std::string & GetName() const override;

    void SetIOType(EPinIOType type) override;
    EPinIOType GetIOType() const override;

    void SetLocation(EPoint2D location) override;
    const EPoint2D & GetLocation() const override;

    void SetPadstackDef(CPtr<IPadstackDef> def) override;
    CPtr<IPadstackDef> GetPadstackDef() const override;

    void SetLayerId(ELayerId lyrId) override;
    ELayerId GetLayerId() const override;

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