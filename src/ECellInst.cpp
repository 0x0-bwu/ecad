#ifndef ECAD_HEADER_ONLY
#include "ECellInst.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECellInst)
#endif

#include "Interface.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ECellInst::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECellInst, ICellInst>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EHierarchyObj);
    ar & boost::serialization::make_nvp("ref_layout", m_refLayout);
    ar & boost::serialization::make_nvp("def_layout", m_defLayout);
}

template <typename Archive>
ECAD_INLINE void ECellInst::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECellInst, ICellInst>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EHierarchyObj);
    ar & boost::serialization::make_nvp("ref_layout", m_refLayout);
    ar & boost::serialization::make_nvp("def_layout", m_defLayout);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECellInst)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ECellInst::ECellInst()
 : ECellInst(std::string{}, nullptr, nullptr)
{
}

ECAD_INLINE ECellInst::ECellInst(std::string name, CPtr<ILayoutView> refLayout, CPtr<ILayoutView> defLayout)
 : EHierarchyObj(std::move(name))
 , m_refLayout(refLayout)
 , m_defLayout(defLayout)
{
}

ECAD_INLINE ECellInst::~ECellInst()
{

}

ECAD_INLINE ECellInst::ECellInst(const ECellInst & other)
{
    *this = other;
}

ECAD_INLINE ECellInst & ECellInst::operator= (const ECellInst & other)
{
    EHierarchyObj::operator=(other);
    m_refLayout = other.m_refLayout;
    m_defLayout = other.m_defLayout;
    return *this;
}

ECAD_INLINE void ECellInst::SetRefLayoutView(CPtr<ILayoutView> refLayout)
{
    m_refLayout = refLayout;
}

ECAD_INLINE CPtr<ILayoutView> ECellInst::GetRefLayoutView() const
{
    return m_refLayout;
}

ECAD_INLINE void ECellInst::SetDefLayoutView(CPtr<ILayoutView> defLayout)
{
    m_defLayout = defLayout;
}

ECAD_INLINE CPtr<ILayoutView> ECellInst::GetDefLayoutView() const
{
    return m_defLayout;
}

ECAD_INLINE CPtr<ILayoutView> ECellInst::GetFlattenedLayoutView() const
{
    return GetDefLayoutView()->GetCell()->GetFlattenedLayoutView();
}


}//namespace ecad