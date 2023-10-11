#include "EDefinition.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EDefinition)

#include "ECadDef.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EDefinition::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EDefinition, IDefinition>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
}

template <typename Archive>
ECAD_INLINE void EDefinition::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EDefinition, IDefinition>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
}
    
ECAD_SERIALIZATION_FUNCTIONS_IMP(EDefinition)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EDefinition::EDefinition()
 : EDefinition(std::string{})
{
}

ECAD_INLINE EDefinition::EDefinition(std::string name)
 : EObject(std::move(name))
{
}

ECAD_INLINE EDefinition::~EDefinition()
{
}

///Copy
ECAD_INLINE EDefinition::EDefinition(const EDefinition & other)
{
    *this = other;
}
ECAD_INLINE EDefinition & EDefinition::operator= (const EDefinition & other)
{
    EObject::operator=(other);
    return *this;
}

ECAD_INLINE EDefinitionType EDefinition::GetDefinitionType() const
{
    return EDefinitionType::Invalid;
}

}//namespace ecad