#include "EMaterialProp.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialProp)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialPropValue)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialPropTable)

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EMaterialProp::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialProp, IMaterialProp>();
}

template <typename Archive>
ECAD_INLINE void EMaterialProp::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialProp, IMaterialProp>();
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EMaterialProp)

template <typename Archive>
ECAD_INLINE void EMaterialPropValue::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialPropValue, IMaterialPropValue>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EMaterialProp);
    ar & boost::serialization::make_nvp("value", m_values);
}

template <typename Archive>
ECAD_INLINE void EMaterialPropValue::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialPropValue, IMaterialPropValue>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EMaterialProp);
    ar & boost::serialization::make_nvp("value", m_values);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EMaterialPropValue)

template <typename Archive>
ECAD_INLINE void EMaterialPropTable::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialPropTable, IMaterialPropTable>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EMaterialProp);
    ar & boost::serialization::make_nvp("denpdency", m_dependency);
    ar & boost::serialization::make_nvp("value", m_values);
}

template <typename Archive>
ECAD_INLINE void EMaterialPropTable::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialPropTable, IMaterialPropTable>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EMaterialProp);
    ar & boost::serialization::make_nvp("denpdency", m_dependency);
    ar & boost::serialization::make_nvp("value", m_values);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EMaterialPropTable)

#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EMaterialPropValue::EMaterialPropValue()
{
    m_values.assign(1, 0);
}

ECAD_INLINE EMaterialPropValue::EMaterialPropValue(EValue value)
{
    m_values.assign(1, value);
}

ECAD_INLINE Ptr<IMaterialPropValue> EMaterialPropValue::GetPropValue()
{
    return dynamic_cast<Ptr<IMaterialPropValue> >(this);
}

ECAD_INLINE void EMaterialPropValue::SetSimpleProperty(const EValue & value)
{
    m_values.assign(1, value);
}

ECAD_INLINE void EMaterialPropValue::SetAnsiotropicProerty(const std::array<EValue, 3> & values)
{
    m_values.resize(3);
    for(size_t i = 0; i < 3; ++i)
        m_values[i] = values[i];
}

ECAD_INLINE void EMaterialPropValue::SetTensorProperty(const std::array<EValue, 9> & values)
{
    m_values.resize(9);
    for(size_t i = 0; i < 9; ++i)
        m_values[i] = values[i];
}

ECAD_INLINE bool EMaterialPropValue::GetSimpleProperty(EValue & value) const
{
    value = m_values.at(0);
    return true; 
}

ECAD_INLINE bool EMaterialPropValue::GetAnsiotropicProperty(size_t row, EValue & value) const
{
    if(row >= 3) return false;
    if(m_values.size() == 1)
        return GetSimpleProperty(value);
    else if(m_values.size() == 3) {
        value = m_values.at(row);
        return true;
    }
    else {
        value = m_values.at(row * 3 + 0);
        return true;
    }
}

ECAD_INLINE bool EMaterialPropValue::GetTensorProperty(size_t row, size_t col, EValue & value) const
{
    if(row >= 3 || col >= 3) return false;
    if(m_values.size() == 1)
        return GetSimpleProperty(value);
    else if(m_values.size() ==3)
        return GetAnsiotropicProperty(row, value);
    else {
        value = m_values.at(row * 3 + col);
        return true;
    }
}

ECAD_INLINE void EMaterialPropValue::GetDimensions(size_t & row, size_t & col) const
{
    if(m_values.size() == 1) {
        row = 1; col = 1; return;
    }
    else if(m_values.size() == 3) {
        row = 3; col = 1; return;
    }
    else {
        row = 3; col = 3; return;
    }
}

ECAD_INLINE EMaterialPropTable::EMaterialPropTable()
 : m_dependency(EMatDependency::Temperature)
{
}

ECAD_INLINE EMaterialPropTable::EMaterialPropTable(EMatDependency dependency)
 : m_dependency(dependency)
{
}

ECAD_INLINE Ptr<IMaterialPropTable> EMaterialPropTable::GetPropTable()
{
    return dynamic_cast<Ptr<IMaterialPropTable> >(this);
}

ECAD_INLINE EMaterialPropTable::EMaterialPropTable(const EMaterialPropTable & other)
{
    *this = other;
}

ECAD_INLINE EMaterialPropTable & EMaterialPropTable::operator= (const EMaterialPropTable & other)
{
    m_dependency = other.m_dependency;
    m_values.clear();
    for(const auto & value : other.m_values){
        m_values.insert(std::make_pair(value.first, CloneHelper(value.second)));
    }
    return *this;  
}

}//namespace ecad