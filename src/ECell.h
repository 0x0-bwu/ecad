#ifndef ECAD_ECELL_H
#define ECAD_ECELL_H
#include "interfaces/ICell.h"
#include "EDefinition.h"
#include "ECadDef.h"
namespace ecad {

class IDatabase;
class ILayoutView;
class ECAD_API ECell : public EDefinition, public ICell
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECell();
    explicit ECell(std::string name, Ptr<IDatabase> database);
    virtual ~ECell();
    
    ///Copy
    ECell(const ECell & other);
    ECell & operator= (const ECell & other);

    virtual void SetDatabase(Ptr<IDatabase> database);
    virtual bool SetLayoutView(UPtr<ILayoutView> layout) { return false; }

    virtual ECellType GetCellType() const;
    virtual Ptr<IDatabase> GetDatabase() const;
    virtual Ptr<ILayoutView> GetLayoutView() const { return nullptr; }
    virtual Ptr<ILayoutView> GetFlattenedLayoutView() { return nullptr; }

    EDefinitionType GetDefinitionType() const;
    const std::string & GetName() const;
    std::string sUuid() const;
    
protected:
    ECellType m_cellType;
    Ptr<IDatabase> m_database;
    UPtr<ILayoutView> m_layoutView;
    UPtr<ILayoutView> m_flattenedLayoutView;
};

class ECAD_API ECircuitCell : public ECell
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECircuitCell();
    explicit ECircuitCell(std::string name, Ptr<IDatabase> database);
    virtual ~ECircuitCell();

    ///Copy
    ECircuitCell(const ECircuitCell & other);
    ECircuitCell & operator= (const ECircuitCell & other);

    bool SetLayoutView(UPtr<ILayoutView> layout);
    Ptr<ILayoutView> GetLayoutView() const;
    Ptr<ILayoutView> GetFlattenedLayoutView();

protected:
    ///Copy
    virtual Ptr<ECircuitCell> CloneImp() const override { return new ECircuitCell(*this); }
};

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

#ifdef ECAD_HEADER_ONLY
#include "ECell.cpp"
#endif

#endif//ECAD_ECELL_H