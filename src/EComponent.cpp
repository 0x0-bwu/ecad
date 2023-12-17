#include "EComponent.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponent)

#include "interfaces/IComponentDef.h"
#include "EShape.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EComponent::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponent, IComponent>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EHierarchyObj);
    ar & boost::serialization::make_nvp("component_def", m_compDef);
    ar & boost::serialization::make_nvp("placement", m_placement);
    ar & boost::serialization::make_nvp("loss_power", m_lossPower);
}

template <typename Archive>
ECAD_INLINE void EComponent::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponent, IComponent>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EHierarchyObj);
    ar & boost::serialization::make_nvp("component_def", m_compDef);
    ar & boost::serialization::make_nvp("placement", m_placement);
    ar & boost::serialization::make_nvp("loss_power", m_lossPower);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponent)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EComponent::EComponent()
 : EComponent(std::string{}, nullptr)
{
}

ECAD_INLINE EComponent::EComponent(std::string name, CPtr<IComponentDef> compDef)
 : EHierarchyObj(std::move(name)), m_compDef(compDef)
{
}

ECAD_INLINE EComponent::~EComponent()
{
}

ECAD_INLINE EComponent::EComponent(const EComponent & other)
{
    *this = other;
}

ECAD_INLINE EComponent & EComponent::operator= (const EComponent & other)
{
    EHierarchyObj::operator=(other);
    m_compDef = other.m_compDef;
    m_placement = other.m_placement;
    m_lossPower = other.m_lossPower;
    return *this;
}

ECAD_INLINE CPtr<IComponentDef> EComponent::GetComponentDef() const
{
    return m_compDef;
}

ECAD_INLINE void EComponent::SetPlacementLayer(ELayerId layer)
{
    m_placement = layer;
}

ECAD_INLINE ELayerId EComponent::GetPlacementLayer() const
{
    return m_placement;
}

ECAD_INLINE void EComponent::AddTransform(const ETransform2D & trans)
{
    EHierarchyObj::AddTransform(trans);
}

ECAD_INLINE void EComponent::SetTransform(const ETransform2D & trans)
{
    EHierarchyObj::SetTransform(trans);
}

ECAD_INLINE const ETransform2D & EComponent::GetTransform() const
{
    return EHierarchyObj::GetTransform();
}

ECAD_INLINE void EComponent::SetLossPower(ESimVal power)
{
    m_lossPower = power;
}

ECAD_INLINE ESimVal EComponent::GetLossPower() const
{
    return m_lossPower;
}

ECAD_INLINE EBox2D EComponent::GetBoundingBox() const
{
    ERectangle bbox(m_compDef->GetBondingBox());
    bbox.Transform(m_transform);
    return bbox.shape;
}

ECAD_INLINE void EComponent::PrintImp(std::ostream & os) const
{
    os << "COMPONENT DEFINE: " << m_compDef->GetName() << ECAD_EOL;
    os << "PLACEMENT: " << m_placement << ECAD_EOL;
    os << "LOSS POWER: " << m_lossPower << 'W' << ECAD_EOL;
    EHierarchyObj::PrintImp(os);
}

}//namespace ecad