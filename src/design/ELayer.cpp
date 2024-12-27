#include "ELayer.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ELayer)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EStackupLayer)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EViaLayer)

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void ELayer::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ELayer, ILayer>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EObject);
    ar & boost::serialization::make_nvp("layer_id", m_layerId);
    ar & boost::serialization::make_nvp("layer_type", m_layerType);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ELayer)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ELayer::ELayer()
 : ELayer(std::string{}, noLayer)
{
}

ELayer::ELayer(std::string name, ELayerId id)
 : EObject(std::move(name))
 , m_layerId(id)
 , m_layerType(ELayerType::Invalid)
{    
}

ELayer::~ELayer()
{
}

void ELayer::SetLayerId(ELayerId id)
{
    m_layerId= id;
}

ELayerId ELayer::GetLayerId() const
{
    return m_layerId;
}

ELayerType ELayer::GetLayerType() const
{
    return m_layerType;
}
    
Ptr<IStackupLayer> ELayer::GetStackupLayerFromLayer()
{
    return dynamic_cast<Ptr<IStackupLayer> >(this);
}

Ptr<IViaLayer> ELayer::GetViaLayerFromLayer()
{
    return dynamic_cast<Ptr<IViaLayer> >(this);   
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
void EStackupLayer::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EStackupLayer, IStackupLayer>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ELayer);
    ar & boost::serialization::make_nvp("elevation", m_elevation);
    ar & boost::serialization::make_nvp("thickness", m_thickness);
    ar & boost::serialization::make_nvp("conducting_mat", m_conductingMat);
    ar & boost::serialization::make_nvp("dielectric_mat", m_dielectricMat);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EStackupLayer)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EStackupLayer::EStackupLayer()
 : EStackupLayer(std::string{}, ELayerType::Invalid)
{
}

EStackupLayer::EStackupLayer(const std::string & name, ELayerType type)
 : ELayer(name)
{
    m_layerType = type;
}

EStackupLayer::~EStackupLayer()
{
}

std::string EStackupLayer::GetName() const
{
    return ELayer::GetName();
}
void EStackupLayer::SetLayerId(ELayerId id)
{
    return ELayer::SetLayerId(id);
}

ELayerId EStackupLayer::GetLayerId() const
{
    return ELayer::GetLayerId();
}

ELayerType EStackupLayer::GetLayerType() const
{
    return ELayer::GetLayerType();
}

void EStackupLayer::SetElevation(EFloat elevation)
{
    m_elevation = elevation;
}

EFloat EStackupLayer::GetElevation() const
{
    return m_elevation;
}

void EStackupLayer::SetThickness(EFloat thickness)
{
    m_thickness = thickness;
}

EFloat EStackupLayer::GetThickness() const
{
    return m_thickness;
}

void EStackupLayer::SetConductingMaterial(const std::string & material)
{
    m_conductingMat = material;
}

const std::string & EStackupLayer::GetConductingMaterial() const
{
    return m_conductingMat;
}

void EStackupLayer::SetDielectricMaterial(const std::string & material)
{
    m_dielectricMat = material;
}

const std::string & EStackupLayer::GetDielectricMaterial() const
{
    return m_dielectricMat;
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void EViaLayer::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EViaLayer, IViaLayer>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ELayer);
    ar & boost::serialization::make_nvp("upper_ref_layer", m_upperRefLyr);
    ar & boost::serialization::make_nvp("lower_ref_layer", m_lowerRefLyr);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EViaLayer)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EViaLayer::EViaLayer()
 : EViaLayer(std::string{})
{
}

EViaLayer::EViaLayer(const std::string & name)
 : ELayer(name)
{
    m_layerType = ELayerType::DielectricLayer;
}

EViaLayer::~EViaLayer()
{
}

}//namesapce ecad