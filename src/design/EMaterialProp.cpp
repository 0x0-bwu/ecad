#include "EMaterialProp.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialProp)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialPropValue)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialPropTable)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EMaterialPropPolynomial)

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

template <typename Archive>
ECAD_INLINE void EMaterialPropPolynomial::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialPropPolynomial, IMaterialPropPolynomial>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EMaterialProp);
    ar & boost::serialization::make_nvp("coefficients", m_coefficients);
}

template <typename Archive>
ECAD_INLINE void EMaterialPropPolynomial::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EMaterialPropPolynomial, IMaterialPropPolynomial>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EMaterialProp);
    ar & boost::serialization::make_nvp("coefficients", m_coefficients);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EMaterialPropPolynomial)

#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EMaterialPropValue::EMaterialPropValue(const std::array<EFloat, 9> & values)
{
    m_values.assign(values.begin(), values.end());
}

ECAD_INLINE EMaterialPropValue::EMaterialPropValue(const std::array<EFloat, 3> & values)
{
    m_values.assign(values.begin(), values.end());
}

ECAD_INLINE EMaterialPropValue::EMaterialPropValue(EFloat value)
{
    m_values.assign(1, value);
}

ECAD_INLINE EMaterialPropValue::EMaterialPropValue()
{
    m_values.assign(1, invalidFloat);
}

ECAD_INLINE Ptr<IMaterialPropValue> EMaterialPropValue::GetPropValue()
{
    return dynamic_cast<Ptr<IMaterialPropValue> >(this);
}

ECAD_INLINE void EMaterialPropValue::SetSimpleProperty(const EFloat & value)
{
    m_values.assign(1, value);
}

ECAD_INLINE void EMaterialPropValue::SetAnsiotropicProerty(const std::array<EFloat, 3> & values)
{
    m_values.resize(3);
    for(size_t i = 0; i < 3; ++i)
        m_values[i] = values[i];
}

ECAD_INLINE void EMaterialPropValue::SetTensorProperty(const std::array<EFloat, 9> & values)
{
    m_values.resize(9);
    for(size_t i = 0; i < 9; ++i)
        m_values[i] = values[i];
}

ECAD_INLINE bool EMaterialPropValue::GetSimpleProperty(EFloat & value) const
{
    value = m_values.at(0);
    return true; 
}

ECAD_INLINE bool EMaterialPropValue::GetAnsiotropicProperty(size_t row, EFloat & value) const
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

ECAD_INLINE bool EMaterialPropValue::GetTensorProperty(size_t row, size_t col, EFloat & value) const
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

ECAD_INLINE bool EMaterialPropValue::GetSimpleProperty([[maybe_unused]] EFloat index, EFloat & value) const
{
    return GetSimpleProperty(value);
}

ECAD_INLINE bool EMaterialPropValue::GetAnsiotropicProperty([[maybe_unused]] EFloat index, size_t row, EFloat & value) const
{
    return GetAnsiotropicProperty(row, value);
}

ECAD_INLINE bool EMaterialPropValue::GetTensorProperty([[maybe_unused]] EFloat index, size_t row, size_t col, EFloat & value) const
{
    return GetTensorProperty(row, col, value);
}

ECAD_INLINE void EMaterialPropValue::GetDimensions(size_t & row, size_t & col) const
{
    if (m_values.size() == 1) {
        row = 1; col = 1; return;
    }
    else if (m_values.size() == 3) {
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

ECAD_INLINE EMaterialPropPolynomial::EMaterialPropPolynomial(std::vector<std::vector<EFloat>> coefficients)
 : m_coefficients(std::move(coefficients))
{
}

ECAD_INLINE EMaterialPropPolynomial::EMaterialPropPolynomial()
{
}

ECAD_INLINE Ptr<IMaterialPropPolynomial> EMaterialPropPolynomial::GetPropPolynomial()
{
    return dynamic_cast<Ptr<IMaterialPropPolynomial> >(this);
}

ECAD_INLINE EFloat EMaterialPropPolynomial::Calculate(const std::vector<EFloat> & coefficients, EFloat index)
{
    ECAD_ASSERT(not coefficients.empty())
    EFloat value = coefficients.front();
    for (size_t i = 1; i < coefficients.size(); ++i)
        value += std::pow(index, i) * coefficients.at(i);
    return value;
}

ECAD_INLINE bool EMaterialPropPolynomial::GetSimpleProperty(EFloat index, EFloat & value) const
{
    const auto & coeffs = m_coefficients.front();
    value = Calculate(coeffs, index);
    return true;
}

ECAD_INLINE bool EMaterialPropPolynomial::GetAnsiotropicProperty(EFloat index, size_t row, EFloat & value) const
{
    const auto & coeffs = 3 == m_coefficients.size() ? m_coefficients.at(row) : m_coefficients.front();
    value = Calculate(coeffs, index);
    return true;
}

ECAD_INLINE bool EMaterialPropPolynomial::GetTensorProperty(EFloat index, size_t row, size_t col, EFloat & value) const
{
    const auto & coeffs = 9 == m_coefficients.size() ? m_coefficients.at(row * 3 + col) :
                        (3 == m_coefficients.size() ? m_coefficients.at(row) : m_coefficients.front());
    value = Calculate(coeffs, index);
    return true;
}

    //1x1-simple, 3x1-anisotropic, 3x3-tensor
ECAD_INLINE void EMaterialPropPolynomial::GetDimensions(size_t & row, size_t & col) const
{
    if (1 == m_coefficients.size()) {
        row = 1; col = 1;
    }
    else if (3 == m_coefficients.size()) {
        row = 3; col = 1;
    }
    else if (9 == m_coefficients.size()) {
        row = 3; col = 3;
    }
    else {
        ECAD_ASSERT(false)
    }
}

}//namespace ecad