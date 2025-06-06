#include "EDefinition.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EDefinition)

#include "interface/IDatabase.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EDefinition::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EDefinition, IDefinition>();
    // ar & boost::serialization::make_nvp("database", m_database); 
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
}
    
ECAD_SERIALIZATION_FUNCTIONS_IMP(EDefinition)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EDefinition::EDefinition()
 : EDefinition(std::string{}, nullptr)
{
}

ECAD_INLINE EDefinition::EDefinition(std::string name, CPtr<IDatabase> database)
 : EObject(std::move(name)), m_database(database)
{
}

ECAD_INLINE EDefinition::~EDefinition()
{
}

ECAD_INLINE void EDefinition::SetDatabase(CPtr<IDatabase> database)
{
    m_database = database;
}

ECAD_INLINE CPtr<IDatabase> EDefinition::GetDatabase() const
{
    return m_database;
}

ECAD_INLINE EDefinitionType EDefinition::GetDefinitionType() const
{
    ECAD_ASSERT(false)
    return EDefinitionType::Invalid;
}

}//namespace ecad