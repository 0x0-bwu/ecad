#pragma once
#include "interfaces/IComponentDef.h"
#include "ECollectionCollection.h"
#include "EDefinition.h"
namespace ecad {

class IDatabase;
class IPadstackDef;
class IComponentDefPin;
class ECAD_API EComponentDef : public EDefinition, public ECollectionCollection, public IComponentDef
{
    ECAD_ALWAYS_INLINE static constexpr std::array<ECollectionType, 1> m_collectionTypes = { ECollectionType::ComponentDefPin };

    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponentDef();
public:
    EComponentDef(std::string name, CPtr<IDatabase> database);
    virtual ~EComponentDef();

    EDefinitionType GetDefinitionType() const override;

    const std::string & GetName() const override;

    void SetComponentType(EComponentType type) override;
    EComponentType GetComponentType() const override;

    void SetBondingBox(const EBox2D & bbox) override;
    const EBox2D & GetBondingBox() const override;

    void SetMaterial(const std::string & name) override;
    const std::string & GetMaterial() const override;

    void SetHeight(EFloat height) override;
    EFloat GetHeight() const override;

    void SetSolderBallBumpHeight(EFloat height) override;
    EFloat GetSolderBallBumpHeight() const override;

    void SetSolderFillingMaterial(const std::string & name) override;
    const std::string & GetSolderFillingMaterial() const override;

    Ptr<IComponentDefPin> CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef = nullptr, ELayerId lyr = noLayer) override;

protected:
    ///Copy
    virtual Ptr<EComponentDef> CloneImp() const override { return new EComponentDef(*this); }
    virtual void PrintImp(std::ostream & os) const override;

protected:
    EComponentType m_type = EComponentType::Invalid;
    EBox2D m_bondingBox;
    EFloat m_height = 0;
    EFloat m_solderHeight = 0;
    std::string m_material;
    std::string m_solderFillingMaterial;
};

ECAD_ALWAYS_INLINE const std::string & EComponentDef::GetName() const
{
    return EDefinition::GetName();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentDef)