#pragma once
#include "interfaces/IComponent.h"
#include "ECadDef.h"
#include "EObject.h"
namespace ecad {

class IComponentDef;
class ECAD_API EComponent : public EObject, public IComponent
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponent();
public:
    EComponent(std::string name, CPtr<IComponentDef> compDef);
    virtual ~EComponent();

    ///Copy
    EComponent(const EComponent & other);
    EComponent & operator= (const EComponent & other);

    const std::string & GetName() const override;

protected:
    ///Copy
    virtual Ptr<EComponent> CloneImp() const override { return new EComponent(*this); }

protected:
    CPtr<IComponentDef> m_compDef;
    EPoint2D m_location;
    ELayerId m_mountLyr;
};

ECAD_ALWAYS_INLINE const std::string & EComponent::GetName() const
{
    return EObject::GetName();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponent)