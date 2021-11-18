#ifndef ECAD_HEADER_ONLY
#include "ECell.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECell)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECircuitCell)
#endif

#include "ELayoutView.h"
#include "EDatabase.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
template <typename Archive>
ECAD_INLINE void ECell::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECell, ICell>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("cell_type", m_cellType);
    ar & boost::serialization::make_nvp("layout_view", m_layoutView);
}

template <typename Archive>
ECAD_INLINE void ECell::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECell, ICell>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EDefinition);
    ar & boost::serialization::make_nvp("cell_type", m_cellType);
    ar & boost::serialization::make_nvp("layout_view", m_layoutView);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECell)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ECell::ECell()
 : ECell(std::string{}, nullptr)
{

}

ECAD_INLINE ECell::ECell(std::string name, Ptr<IDatabase> database)
 : EDefinition(std::move(name))
 , m_cellType(ECellType::Invalid)
 , m_database(database)
 , m_layoutView(nullptr)
{  
  
}

ECAD_INLINE ECell::~ECell()
{

}

ECAD_INLINE ECell::ECell(const ECell & other)
{
    *this = other;
}

ECAD_INLINE ECell & ECell::operator= (const ECell & other)
{
    EDefinition::operator=(other);
    m_cellType = other.m_cellType;
    m_database = other.m_database;
    m_layoutView = CloneHelper(other.m_layoutView);
    m_layoutView->SetCell(this);
    m_flattenedLayoutView = CloneHelper(other.m_flattenedLayoutView);
    if(m_flattenedLayoutView)
        m_flattenedLayoutView->SetCell(this);
}

ECAD_INLINE ECellType ECell::GetCellType() const
{
    return m_cellType;
}

ECAD_INLINE void ECell::SetDatabase(Ptr<IDatabase> database)
{
    m_database = database; 
}

ECAD_INLINE Ptr<IDatabase> ECell::GetDatabase() const
{
    return m_database;
}

ECAD_INLINE EDefinitionType ECell::GetDefinitionType() const
{
    return EDefinitionType::Cell;
}

ECAD_INLINE ECircuitCell::ECircuitCell()
 : ECircuitCell(std::string{}, nullptr)
{
}

ECAD_INLINE ECircuitCell::ECircuitCell(std::string name, Ptr<IDatabase> database)
 : ECell(name, database)
{    
    m_cellType = ECellType::CircuitCell;
    m_layoutView.reset(new ELayoutView(name, this));
}

ECAD_INLINE ECircuitCell::~ECircuitCell()
{
}

ECAD_INLINE ECircuitCell::ECircuitCell(const ECircuitCell & other)
{
    *this = other;
}

ECAD_INLINE ECircuitCell & ECircuitCell::operator= (const ECircuitCell & other)
{
    ECell::operator=(other);
    return *this;
}

ECAD_INLINE bool ECircuitCell::SetLayoutView(UPtr<ILayoutView> layout)
{
    m_layoutView = std::move(layout);
    m_layoutView->SetCell(this);
    return true;
}

ECAD_INLINE Ptr<ILayoutView> ECircuitCell::GetLayoutView() const
{
    return m_layoutView.get();
}

ECAD_INLINE Ptr<ILayoutView> ECircuitCell::GetFlattenedLayoutView()
{
    if(nullptr == m_flattenedLayoutView){
        m_flattenedLayoutView = CloneHelper(m_layoutView);
        m_flattenedLayoutView->Flatten(EFlattenOption{});
    }
    return m_flattenedLayoutView.get();
}
}//namesapce ecad