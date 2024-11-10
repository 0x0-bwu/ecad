#pragma once
#include "interface/IComponentDef.h"
#include "collection/ECollectionCollection.h"
#include "EDefinition.h"
namespace ecad {

class IDatabase;
class IPadstackDef;
class IComponentDefPin;
class ECAD_API EComponentDef : public EDefinition, public ECollectionCollection, public IComponentDef
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EComponentDef();
public:
    EComponentDef(std::string name, CPtr<IDatabase> database);
    virtual ~EComponentDef();

    ///Copy
    EComponentDef(const EComponentDef & other);
    EComponentDef & operator= (const EComponentDef & other);

    void SetDatabase(CPtr<IDatabase> database) override;
    CPtr<IDatabase> GetDatabase() const override;

    EDefinitionType GetDefinitionType() const override;

    const std::string & GetName() const override;

    void SetComponentType(EComponentType type) override;
    EComponentType GetComponentType() const override;

    void SetBoundary(UPtr<EShape> boundary) override;
    CPtr<EShape> GetBoundary() const override;

    void SetMaterial(const std::string & name) override;
    const std::string & GetMaterial() const override;

    void SetHeight(EFloat height) override;
    EFloat GetHeight() const override;

    void SetSolderBallBumpHeight(EFloat height) override;
    EFloat GetSolderBallBumpHeight() const override;

    void SetSolderFillingMaterial(const std::string & name) override;
    const std::string & GetSolderFillingMaterial() const override;

    Ptr<IComponentDefPin> CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef = nullptr, ELayerId lyr = noLayer) override;
    Ptr<IComponentDefPin> FindPinByName(const std::string & name) const override;
protected:
    virtual ECollectionTypes GetCollectionTypes() const override { return { ECollectionType::ComponentDefPin }; }
    virtual Ptr<EComponentDef> CloneImp() const override { return new EComponentDef(*this); }
    virtual void PrintImp(std::ostream & os) const override;

protected:
    EComponentType m_type = EComponentType::Invalid;
    UPtr<EShape> m_boundary;
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