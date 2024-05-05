#include "ECellInst.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECellInst)

#include "interface/ILayoutView.h"
#include "interface/ILayerMap.h"
#include "interface/ICell.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void ECellInst::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECellInst, ICellInst>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EHierarchyObj);
    ar & boost::serialization::make_nvp("def_layout", m_defLayout);
    ar & boost::serialization::make_nvp("layer_map", m_layerMap);
}

template <typename Archive>
ECAD_INLINE void ECellInst::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECellInst, ICellInst>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EHierarchyObj);
    ar & boost::serialization::make_nvp("def_layout", m_defLayout);
    ar & boost::serialization::make_nvp("layer_map", m_layerMap);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECellInst)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE ECellInst::ECellInst()
 : ECellInst(std::string{}, nullptr, nullptr)
{
}

ECAD_INLINE ECellInst::ECellInst(std::string name, CPtr<ILayoutView> refLayout, CPtr<ILayoutView> defLayout)
 : EHierarchyObj(std::move(name), refLayout)
 , m_defLayout(defLayout)
{
}

ECAD_INLINE ECellInst::~ECellInst()
{
}

ECAD_INLINE void ECellInst::SetName(std::string name)
{
    EHierarchyObj::SetName(std::move(name));
}

ECAD_INLINE const std::string & ECellInst::GetName() const
{
    return EHierarchyObj::GetName();
}

ECAD_INLINE void ECellInst::SetRefLayoutView(CPtr<ILayoutView> refLayout)
{
    EHierarchyObj::SetRefLayoutView(refLayout);
}

ECAD_INLINE CPtr<ILayoutView> ECellInst::GetRefLayoutView() const
{
    return EHierarchyObj::GetRefLayoutView();
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

ECAD_INLINE void ECellInst::SetLayerMap(CPtr<ILayerMap> layerMap)
{
    m_layerMap = layerMap;
}

ECAD_INLINE CPtr<ILayerMap> ECellInst::GetLayerMap() const
{
    return m_layerMap;
}


}//namespace ecad