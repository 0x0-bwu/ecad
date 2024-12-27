#include "ECell.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECell)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECircuitCell)

#include "ELayoutView.h"
#include "EDatabase.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void ECell::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECell, ICell>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("cell_type", m_cellType);
    ar & boost::serialization::make_nvp("layout_view", m_layoutView);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECell)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECell::ECell()
 : ECell(std::string{}, nullptr)
{

}

ECell::ECell(std::string name, CPtr<IDatabase> database)
 : EDefinition(std::move(name), database)
 , m_cellType(ECellType::Invalid)
 , m_layoutView(nullptr)
{  
  
}

ECell::~ECell()
{

}

ECell::ECell(const ECell & other)
{
    *this = other;
}

ECell & ECell::operator= (const ECell & other)
{
    EDefinition::operator=(other);
    m_cellType = other.m_cellType;
    m_layoutView = CloneHelper(other.m_layoutView);
    m_layoutView->SetCell(this);
    return *this;
}

ECellType ECell::GetCellType() const
{
    return m_cellType;
}

const ECoordUnits & ECell::GetCoordUnits() const
{
    return GetDatabase()->GetCoordUnits();
}

void ECell::SetDatabase(CPtr<IDatabase> database)
{
    return EDefinition::SetDatabase(database);
}

CPtr<IDatabase> ECell::GetDatabase() const
{
    return EDefinition::GetDatabase();
}

EDefinitionType ECell::GetDefinitionType() const
{
    return EDefinitionType::Cell;
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void ECircuitCell::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECell);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECircuitCell)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECircuitCell::ECircuitCell()
 : ECircuitCell(std::string{}, nullptr)
{
}

ECircuitCell::ECircuitCell(std::string name, CPtr<IDatabase> database)
 : ECell(name, database)
{    
    m_cellType = ECellType::CircuitCell;
    m_layoutView.reset(new ELayoutView(name, this));
}

ECircuitCell::~ECircuitCell()
{
}

ECircuitCell::ECircuitCell(const ECircuitCell & other)
{
    *this = other;
}

ECircuitCell & ECircuitCell::operator= (const ECircuitCell & other)
{
    ECell::operator=(other);
    return *this;
}

bool ECircuitCell::SetLayoutView(UPtr<ILayoutView> layout)
{
    m_layoutView = std::move(layout);
    m_layoutView->SetCell(this);
    return true;
}

Ptr<ILayoutView> ECircuitCell::GetLayoutView() const
{
    return m_layoutView.get();
}

Ptr<ILayoutView> ECircuitCell::GetFlattenedLayoutView()
{
    m_layoutView->Flatten(EFlattenOption{});
    return m_layoutView.get();
}

}//namesapce ecad