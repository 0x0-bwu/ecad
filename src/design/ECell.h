#pragma once
#include "interface/ICell.h"
#include "EDefinition.h"
namespace ecad {

class IDatabase;
class ILayoutView;
class ECAD_API ECell : public EDefinition, public ICell
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECell();
    explicit ECell(std::string name, CPtr<IDatabase> database);
    virtual ~ECell();
    
    ///Copy
    ECell(const ECell & other);
    ECell & operator= (const ECell & other);

    const ECoordUnits & GetCoordUnits() const override;

    void SetDatabase(CPtr<IDatabase> database) override;
    CPtr<IDatabase> GetDatabase() const override;
    
    virtual bool SetLayoutView(UPtr<ILayoutView> layout) override { return false; }

    virtual ECellType GetCellType() const override;
    virtual Ptr<ILayoutView> GetLayoutView() const override { return nullptr; }
    virtual Ptr<ILayoutView> GetFlattenedLayoutView() override { return nullptr; }

    EDefinitionType GetDefinitionType() const override;
    void Print(std::ostream & os) const override;
    const std::string & GetName() const override;
    std::string sUuid() const override;
    
protected:
    ECellType m_cellType;
    UPtr<ILayoutView> m_layoutView;
};

class ECAD_API ECircuitCell : public ECell
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECircuitCell();
    explicit ECircuitCell(std::string name, CPtr<IDatabase> database);
    virtual ~ECircuitCell();

    ///Copy
    ECircuitCell(const ECircuitCell & other);
    ECircuitCell & operator= (const ECircuitCell & other);

    bool SetLayoutView(UPtr<ILayoutView> layout) override;
    Ptr<ILayoutView> GetLayoutView() const override;
    Ptr<ILayoutView> GetFlattenedLayoutView() override;

protected:
    ///Copy
    virtual Ptr<ECircuitCell> CloneImp() const override { return new ECircuitCell(*this); }
    virtual void PrintImp(std::ostream & os) const override { ECell::Print(os); }
};

ECAD_ALWAYS_INLINE void ECell::Print(std::ostream & os) const
{
    return EDefinition::Print(os);
}

ECAD_ALWAYS_INLINE const std::string & ECell::GetName() const
{
    return EDefinition::GetName();
}

ECAD_ALWAYS_INLINE std::string ECell::sUuid() const
{
    return EDefinition::sUuid();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECell)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECircuitCell)