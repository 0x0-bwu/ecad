#pragma once
#include "interfaces/IComponent.h"
#include "EHierarchyObj.h"
namespace ecad {

class IComponentDef;
class ECAD_API EComponent : public EHierarchyObj, public IComponent
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponent();
public:
    EComponent(std::string name, CPtr<IComponentDef> compDef);
    virtual ~EComponent();

    ///Copy
    EComponent(const EComponent & other);
    EComponent & operator= (const EComponent & other);

    void SetPlacementLayer(ELayerId layer) override;
    ELayerId GetPlacementLayer() const override;

    void SetTransform(const ETransform2D & trans) override;
    const ETransform2D & GetTransform() const override;

    const std::string & GetName() const override;
protected:
    ///Copy
    virtual Ptr<EComponent> CloneImp() const override { return new EComponent(*this); }
protected:
    CPtr<IComponentDef> m_compDef;
    ELayerId m_placement{ELayerId::noLayer};
};

ECAD_ALWAYS_INLINE void EComponent::SetPlacementLayer(ELayerId layer)
{
    m_placement = layer;
}

ECAD_ALWAYS_INLINE ELayerId EComponent::GetPlacementLayer() const
{
    return m_placement;
}

ECAD_ALWAYS_INLINE void EComponent::SetTransform(const ETransform2D & trans)
{
    EHierarchyObj::SetTransform(trans);
}

ECAD_ALWAYS_INLINE const ETransform2D & EComponent::GetTransform() const
{
    return EHierarchyObj::GetTransform();
}

ECAD_ALWAYS_INLINE const std::string & EComponent::GetName() const
{
    return EHierarchyObj::GetName();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponent)