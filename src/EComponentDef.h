#pragma once
#include "interfaces/IComponentDef.h"
#include "ECollectionCollection.h"
#include "EDefinition.h"
namespace ecad {

class IPadstackDef;
class IComponentDefPin;
class ECAD_API EComponentDef : public EDefinition, public ECollectionCollection, public IComponentDef
{
    ECAD_ALWAYS_INLINE static constexpr std::array<ECollectionType, 1> m_collectionTypes = { ECollectionType::ComponentDefPin };

    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponentDef();
public:
    EComponentDef(const std::string & name);
    virtual ~EComponentDef();

    ///Copy
    EComponentDef(const EComponentDef & other);
    EComponentDef & operator= (const EComponentDef & other);

    EDefinitionType GetDefinitionType() const override;

    const std::string & GetName() const override;

    void SetComponentType(EComponentType type) override;
    EComponentType GetComponentType() const override;

    void SetBondingBox(const EBox2D & bbox) override;
    const EBox2D & GetBondingBox() const override;

    void SetMaterial(const std::string & name) override;
    const std::string & GetMaterial() const override;

    void SetHeight(FCoord height) override;
    FCoord GetHeight() const override;

    Ptr<IComponentDefPin> CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef = nullptr, ELayerId lyr = noLayer) override;

protected:
    ///Copy
    virtual Ptr<EComponentDef> CloneImp() const override { return new EComponentDef(*this); }
    virtual void PrintImp(std::ostream & os) const override;

protected:
    EComponentType m_type = EComponentType::Invalid;
    EBox2D m_bondingBox;
    FCoord m_height = 0;
    std::string m_material;
};

ECAD_ALWAYS_INLINE const std::string & EComponentDef::GetName() const
{
    return EDefinition::GetName();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentDef)