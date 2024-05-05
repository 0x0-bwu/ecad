#include "EHierarchyObj.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EHierarchyObj)

#include "generic/math/MathIO.hpp"
#include "interface/ILayoutView.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EHierarchyObj::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EHierarchyObj, IHierarchyObj>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("ref_layout", m_refLayout);
    ar & boost::serialization::make_nvp("transform", m_transform);
}

template <typename Archive>
ECAD_INLINE void EHierarchyObj::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EHierarchyObj, IHierarchyObj>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("ref_layout", m_refLayout);
    ar & boost::serialization::make_nvp("transform", m_transform);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EHierarchyObj)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EHierarchyObj::EHierarchyObj()
 : EHierarchyObj(std::string{}, nullptr)
{
}

ECAD_INLINE EHierarchyObj::EHierarchyObj(std::string name, CPtr<ILayoutView> refLayout)
 : EObject(name), m_refLayout(refLayout)
{
}

ECAD_INLINE EHierarchyObj::~EHierarchyObj()
{
}

ECAD_INLINE void EHierarchyObj::SetRefLayoutView(CPtr<ILayoutView> refLayout)
{
    m_refLayout = refLayout;
}

ECAD_INLINE CPtr<ILayoutView> EHierarchyObj::GetRefLayoutView() const
{
    return m_refLayout;
}

ECAD_INLINE void EHierarchyObj::PrintImp(std::ostream & os) const
{
    os << "TRANSFORM: " << m_transform.GetTransform() << ECAD_EOL;
}

}//namespace ecad