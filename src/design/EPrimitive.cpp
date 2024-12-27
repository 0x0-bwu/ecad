#include "EPrimitive.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPrimitive)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EGeometry2D)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EBondwire)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EText)

#include "interface/IPadstackDef.h"
#include "interface/IComponent.h"
#include "interface/ILayer.h"
#include "interface/INet.h"
#include "basic/EShape.h"

#include "generic/geometry/Utility.hpp"

namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void EPrimitive::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPrimitive, IPrimitive>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EConnObj);
    ar & boost::serialization::make_nvp("layer", m_layer);
    ar & boost::serialization::make_nvp("type", m_type);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPrimitive)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EPrimitive::EPrimitive()
 : EPrimitive(std::string{}, noLayer, noNet)
{
}

EPrimitive::EPrimitive(std::string name, ELayerId layer, ENetId net)
 : EConnObj(std::move(name), net)
 , m_layer(layer)
 , m_type(EPrimitiveType::Invalid)
{
}

EPrimitive::EPrimitive(ENetId net)
 : EPrimitive(std::string{}, noLayer, net)
{
}

EPrimitive::~EPrimitive()
{
}

EPrimitive::EPrimitive(const EPrimitive & other)
{
    *this = other;
}

EPrimitive & EPrimitive::operator= (const EPrimitive & other)
{
    EConnObj::operator=(other);
    m_layer = other.m_layer;
    m_type = other.m_type;
    return *this;
}

Ptr<IText> EPrimitive::GetTextFromPrimitive()
{
    return dynamic_cast<Ptr<IText> >(this);  
}

Ptr<IConnObj> EPrimitive::GetConnObjFromPrimitive()
{
    return dynamic_cast<Ptr<IConnObj> >(this);
}

Ptr<IGeometry2D> EPrimitive::GetGeometry2DFromPrimitive()
{
    if (m_type != EPrimitiveType::Geometry2D) return nullptr;
    return dynamic_cast<Ptr<IGeometry2D> >(this);
}

Ptr<IBondwire> EPrimitive::GetBondwireFromPrimitive()
{
    if (m_type != EPrimitiveType::Bondwire) return nullptr;
    return dynamic_cast<Ptr<IBondwire> >(this);
}

void EPrimitive::SetNet(ENetId net)
{
    EConnObj::SetNet(net);
}

ENetId EPrimitive::GetNet() const
{
    return EConnObj::GetNet();
}

void EPrimitive::SetLayer(ELayerId layer)
{
    m_layer = layer;
}

ELayerId EPrimitive::GetLayer() const
{
    return m_layer;
}

EPrimitiveType EPrimitive::GetPrimitiveType() const
{
    return m_type;
}

void EPrimitive::PrintImp(std::ostream & os) const
{
    os << "LAYER: " << m_layer << ECAD_EOL;
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void EGeometry2D::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EGeometry2D, IGeometry2D>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EPrimitive);
    ar & boost::serialization::make_nvp("shape", m_shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EGeometry2D)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EGeometry2D::EGeometry2D()
 : EGeometry2D(noLayer, noNet, nullptr)
{
}

EGeometry2D::EGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape)
 : EPrimitive("", layer, net)
 , m_shape(std::move(shape))
{
    m_type = EPrimitiveType::Geometry2D;
}

EGeometry2D::EGeometry2D(ELayerId layer, ENetId net)
 : EGeometry2D(layer, net, nullptr)
{
}

EGeometry2D::~EGeometry2D()
{
}

EGeometry2D::EGeometry2D(const EGeometry2D & other)
{
    *this = other;
}

EGeometry2D & EGeometry2D::operator= (const EGeometry2D & other)
{
    EPrimitive::operator=(other);
    m_shape = CloneHelper(other.m_shape);
    return *this;
}

void EGeometry2D::SetShape(UPtr<EShape> shape)
{
    m_shape = std::move(shape);
}

Ptr<EShape> EGeometry2D::GetShape() const
{
    return m_shape.get();
}

void EGeometry2D::Transform(const ETransform2D & transform)
{
    if(m_shape) m_shape->Transform(transform);
}

void EGeometry2D::PrintImp(std::ostream & os) const
{
    os << "TYPE: " << "GEOMETRY2D" << ECAD_EOL;
    os << *m_shape << ECAD_EOL;
    EPrimitive::PrintImp(os);
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void EBondwire::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EBondwire, IBondwire>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EPrimitive);
    ar & boost::serialization::make_nvp("material", m_material);
    ar & boost::serialization::make_nvp("end_layer", m_endLayer);
    ar & boost::serialization::make_nvp("mount_component", m_mountComp);
    ar & boost::serialization::make_nvp("connected_pin", m_connectedPin);
    ar & boost::serialization::make_nvp("location", m_location);
    ar & boost::serialization::make_nvp("flipped", m_flipped);
    ar & boost::serialization::make_nvp("radius", m_radius);
    ar & boost::serialization::make_nvp("height", m_height);
    ar & boost::serialization::make_nvp("current", m_current);
    ar & boost::serialization::make_nvp("scenario", m_scenario);
    ar & boost::serialization::make_nvp("solder_joints", m_solderJoints);
    ar & boost::serialization::make_nvp("bondwire_type", m_bondwireType);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EBondwire)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EBondwire::EBondwire()
 : EBondwire(std::string{}, noNet, 0)
{
}

EBondwire::EBondwire(std::string name, ENetId net, EFloat radius)
 : EPrimitive(std::move(name), ELayerId::noLayer, net), m_radius(radius)
{
    m_type = EPrimitiveType::Bondwire;
}

const std::string & EBondwire::GetName() const
{
    return EPrimitive::GetName();
}

void EBondwire::SetNet(ENetId net)
{
    return EPrimitive::SetNet(net);
}

ENetId EBondwire::GetNet() const
{
    return EPrimitive::GetNet();
}

void EBondwire::SetRadius(EFloat r)
{
    m_radius = r;
}

EFloat EBondwire::GetRadius() const
{
    return m_radius;
}

EPoint2D EBondwire::GetStartPt() const
{
    if (m_mountComp.front() && not m_connectedPin.front().empty()) {
        if (EPoint2D location; m_mountComp.front()->GetPinLocation(m_connectedPin.front(), location))
            return location;
    }
    return m_location.front();
}

EPoint2D EBondwire::GetEndPt() const
{
    if (m_mountComp.back() && not m_connectedPin.back().empty()) {
        if (EPoint2D location; m_mountComp.back()->GetPinLocation(m_connectedPin.back(), location))
            return location;
    }
    return m_location.back();
}

const std::string & EBondwire::GetStartComponentPin() const
{
    return m_connectedPin.front();
}

const std::string & EBondwire::GetEndComponentPin() const
{
    return m_connectedPin.back();
}

void EBondwire::SetStartLayer(ELayerId layerId, const EPoint2D & location, bool flipped)
{
    m_location.front() = location;
    m_flipped.front() = flipped;
    return SetStartLayer(layerId);
}

void EBondwire::SetStartLayer(ELayerId layerId)
{
    m_connectedPin.front().clear();
    m_mountComp.front() = nullptr;
    return EPrimitive::SetLayer(layerId); 
}

ELayerId EBondwire::GetStartLayer(Ptr<bool> flipped) const
{
    if(flipped) *flipped = ELayerId::ComponentLayer == m_layer ? m_mountComp.front()->isFlipped() : m_flipped.front();
    return EPrimitive::GetLayer();
}

void EBondwire::SetEndLayer(ELayerId layerId, const EPoint2D & location, bool flipped)
{
    m_location.back() = location;
    m_flipped.back() = flipped;
    return SetEndLayer(layerId);
}

void EBondwire::SetEndLayer(ELayerId layerId)
{
    m_connectedPin.back().clear();
    m_mountComp.back() = nullptr;
    m_endLayer = layerId;
}

ELayerId EBondwire::GetEndLayer(Ptr<bool> flipped) const
{
    if(flipped) *flipped = ELayerId::ComponentLayer == m_endLayer ? m_mountComp.back()->isFlipped() : m_flipped.back();
    return m_endLayer;
}

void EBondwire::SetMaterial(const std::string & material)
{
    m_material = material;
}

const std::string & EBondwire::GetMaterial() const
{
    return m_material;
}

void EBondwire::SetBondwireType(EBondwireType type)
{
    m_bondwireType = type;
}

EBondwireType EBondwire::GetBondwireType() const
{
    return m_bondwireType;
}

void EBondwire::SetHeight(EFloat height)
{
    m_height = height;
}

EFloat EBondwire::GetHeight() const
{
    return m_height;
}

void EBondwire::SetStartComponent(CPtr<IComponent> comp, const std::string & pin)
{
    m_layer = ELayerId::ComponentLayer;
    m_connectedPin.front() = pin;
    m_mountComp.front() = comp;
}

CPtr<IComponent> EBondwire::GetStartComponent() const
{
    return m_mountComp.front();
}

void EBondwire::SetEndComponent(CPtr<IComponent> comp, const std::string & pin)
{
    m_endLayer = ELayerId::ComponentLayer;
    m_connectedPin.back() = pin;
    m_mountComp.back() = comp;
}

CPtr<IComponent> EBondwire::GetEndComponent() const
{
    return m_mountComp.back();
}

void EBondwire::SetSolderJoints(CPtr<IPadstackDef> s)
{
    m_solderJoints = s;
}

CPtr<IPadstackDef> EBondwire::GetSolderJoints() const
{
    return m_solderJoints;
}

void EBondwire::SetCurrent(EFloat current)
{
    m_current = current;
}

EFloat EBondwire::GetCurrent() const
{
    return m_current;
}

void EBondwire::SetDynamicPowerScenario(EScenarioId id)
{
    m_scenario = id;
}

EScenarioId EBondwire::GetDynamicPowerScenario() const
{
    return m_scenario;
}

void EBondwire::Transform(const ETransform2D & transform)
{
    auto trans = transform.GetTransform();
    generic::geometry::Transform(m_location.front(), trans);
    generic::geometry::Transform(m_location.back(), trans);
}

void EBondwire::PrintImp(std::ostream & os) const
{
    os << "TYPE: " << "BONDWIRE" << ECAD_EOL;
    os << "START: " << m_location.front() << ", END: " << m_location.back() << ECAD_EOL;
    os << "RADIUS: " << m_radius << ECAD_EOL;
    os << "HEIGHT: " << m_height << ECAD_EOL;
    os << "MATERIAL: " << m_material << ECAD_EOL;
    EPrimitive::PrintImp(os);
}

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void EText::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EText, IText>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EPrimitive);
    ar & boost::serialization::make_nvp("text", m_text);
    ar & boost::serialization::make_nvp("transform", m_transform);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EText)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

EText::EText()
 : EText(std::string{}, noLayer, noNet)
{
}

EText::EText(std::string text, ELayerId layer, ENetId net)
 : EPrimitive("", layer, net)
 , m_text(std::move(text))
{
    m_type = EPrimitiveType::Text;
}

EText::EText(std::string text)
 : EText(std::move(text), noLayer, noNet)
{
}

EText::~EText()
{
}

const std::string & EText::GetText() const
{
    return m_text;
}

EPoint2D EText::GetPosition() const
{
    auto transform = m_transform.GetTransform();
    return EPoint2D(transform(0, 2), transform(1, 2));
}

}//namespace ecad