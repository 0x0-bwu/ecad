#include "EComponent.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EComponent)

#include "interface/IComponentDefPin.h"
#include "interface/IComponentDef.h"
#include "basic/EShape.h"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EComponent::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EComponent, IComponent>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EHierarchyObj);
    ar & boost::serialization::make_nvp("component_def", m_compDef);
    ar & boost::serialization::make_nvp("placement", m_placement);
    ar & boost::serialization::make_nvp("loss_power", m_lossPower);
    ar & boost::serialization::make_nvp("flipped", m_flipped);
    ar & boost::serialization::make_nvp("scenario", m_scenario);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EComponent)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EComponent::EComponent()
 : EComponent(std::string{}, nullptr, nullptr)
{
}

ECAD_INLINE EComponent::EComponent(std::string name, CPtr<ILayoutView> refLayout, CPtr<IComponentDef> compDef)
 : EHierarchyObj(std::move(name), refLayout), m_compDef(compDef)
{
}

ECAD_INLINE EComponent::~EComponent()
{
}

ECAD_INLINE CPtr<IComponentDef> EComponent::GetComponentDef() const
{
    return m_compDef;
}

ECAD_INLINE CPtr<ILayoutView> EComponent::GetRefLayoutView() const
{
    return EHierarchyObj::GetRefLayoutView();
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

ECAD_INLINE bool EComponent::hasLossPower() const
{
    return not m_lossPower.Empty();
}

ECAD_INLINE void EComponent::SetLossPower(EFloat kelvin, EFloat power)
{
    m_lossPower.AddSample(kelvin, power);
}

ECAD_INLINE EFloat EComponent::GetLossPower(EFloat kelvin) const
{
    return m_lossPower.Lookup(kelvin);
}

ECAD_INLINE const ELookupTable1D & EComponent::GetLossPowerTable() const
{
    return m_lossPower;
}

ECAD_INLINE void EComponent::SetDynamicPowerScenario(EScenarioId id)
{
    m_scenario = id;
}

ECAD_INLINE EScenarioId EComponent::GetDynamicPowerScenario() const
{
    return m_scenario;
}

ECAD_INLINE UPtr<EShape> EComponent::GetBoundary() const
{
    if (auto bond = m_compDef->GetBoundary(); bond) {
        auto clone = bond->Clone();
        auto trans = ETransform2D{};
        if (m_flipped) 
            trans.Mirror() = EMirror2D::XY;
        trans.Append(m_transform);
        clone->Transform(trans);
        return clone;
    }
    return nullptr;
}

ECAD_INLINE void EComponent::SetFlipped(bool flipped)
{
    m_flipped = flipped;
}

ECAD_INLINE bool EComponent::isFlipped() const
{
    return m_flipped;
}

ECAD_INLINE EFloat EComponent::GetHeight() const
{
    return m_compDef->GetHeight();
}

ECAD_INLINE bool EComponent::GetPinLocation(const std::string & name, EPoint2D & loc) const
{
    auto pin = m_compDef->FindPinByName(name);
    if (nullptr == pin) return false;
    ETransform2D trans;
    if (m_flipped)
        trans.Mirror() = EMirror2D::XY;
    trans.Append(m_transform);
    loc = trans.GetTransform() * pin->GetLocation();
    return true; 
}

ECAD_INLINE void EComponent::PrintImp(std::ostream & os) const
{
    os << "COMPONENT DEFINE: " << m_compDef->GetName() << ECAD_EOL;
    os << "PLACEMENT: " << m_placement << ECAD_EOL;
    os << "FLIPPED: " << std::boolalpha << m_flipped << ECAD_EOL;
    os << "LOSS POWER: " << m_lossPower << ECAD_EOL;
    EHierarchyObj::PrintImp(os);
}

}//namespace ecad