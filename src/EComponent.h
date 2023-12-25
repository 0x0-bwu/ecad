#pragma once
#include "interfaces/IComponent.h"
#include "EHierarchyObj.h"
namespace ecad {

class ECAD_API EComponent : public EHierarchyObj, public IComponent
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponent();
public:
    EComponent(std::string name, CPtr<ILayoutView> refLayout, CPtr<IComponentDef> compDef);
    virtual ~EComponent();

    CPtr<IComponentDef> GetComponentDef() const override;
    
    CPtr<ILayoutView> GetRefLayoutView() const override;

    void SetPlacementLayer(ELayerId layer) override;
    ELayerId GetPlacementLayer() const override;

    void AddTransform(const ETransform2D & trans) override;
    void SetTransform(const ETransform2D & trans) override;
    const ETransform2D & GetTransform() const override;

    void SetLossPower(ESimVal power) override;
    ESimVal GetLossPower() const override;

    EBox2D GetBoundingBox() const override;

    void SetFlipped(bool flipped) override;
    bool isFlipped() const override;

    FCoord GetHeight() const override;

    const std::string & GetName() const override;
protected:
    ///Copy
    virtual Ptr<EComponent> CloneImp() const override { return new EComponent(*this); }
    virtual void PrintImp(std::ostream & os) const override;
protected:
    CPtr<IComponentDef> m_compDef;
    ELayerId m_placement{ELayerId::noLayer};
    ESimVal m_lossPower{0};
    bool m_flipped{false};
};

ECAD_ALWAYS_INLINE const std::string & EComponent::GetName() const
{
    return EHierarchyObj::GetName();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponent)