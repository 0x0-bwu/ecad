#pragma once
#include "interfaces/IComponent.h"
#include "EHierarchyObj.h"
#include "ELookupTable.h"
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

    bool hasLossPower() const override;
    void SetLossPower(EFloat kelvin, EFloat power) override;
    EFloat GetLossPower(EFloat kelvin) const override;
    const ELookupTable1D & GetLossPowerTable() const override;

    EBox2D GetBoundingBox() const override;

    void SetFlipped(bool flipped) override;
    bool isFlipped() const override;

    EFloat GetHeight() const override;

    bool GetPinLocation(const std::string & name, EPoint2D & loc) const override;

    void SetName(std::string name) override;
    const std::string & GetName() const override;
protected:
    ///Copy
    virtual Ptr<EComponent> CloneImp() const override { return new EComponent(*this); }
    virtual void PrintImp(std::ostream & os) const override;
protected:
    CPtr<IComponentDef> m_compDef;
    ELayerId m_placement{ELayerId::noLayer};
    ELookupTable1D m_lossPower;
    bool m_flipped{false};
};

ECAD_ALWAYS_INLINE void EComponent::SetName(std::string name)
{
    return EHierarchyObj::SetName(std::move(name));
}

ECAD_ALWAYS_INLINE const std::string & EComponent::GetName() const
{
    return EHierarchyObj::GetName();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponent)