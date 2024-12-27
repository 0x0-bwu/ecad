#include "EShape.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EShapeFromTemplate)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPolygonWithHoles)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ERectangle)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPolygon)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ECircle)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPath)

#include "generic/geometry/Utility.hpp"
#include "ETransform.h"
namespace ecad {

using namespace generic;
using namespace generic::geometry;

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
void ERectangle::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ERectangle, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ERectangle)

template <typename Archive>
void EPath::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPath, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("width", m_width);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPath)

template <typename Archive>
void ECircle::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ECircle, EShape>();
    ar & boost::serialization::make_nvp("o", o);
    ar & boost::serialization::make_nvp("r", r);
    ar & boost::serialization::make_nvp("div", div);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECircle)

template <typename Archive>
void EPolygon::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygon, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPolygon)

template <typename Archive>
void EPolygonWithHoles::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygonWithHoles, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPolygonWithHoles)

template <typename Archive>
void EShapeFromTemplate::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EShapeFromTemplate, EShape>();
    ar & boost::serialization::make_nvp("template", m_template);
    ar & boost::serialization::make_nvp("transform", m_transform);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EShapeFromTemplate)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

bool ERectangle::hasHole() const
{
    return false; 
}

EBox2D ERectangle::GetBBox() const
{
    return shape;
}

EPolygonData ERectangle::GetContour() const
{
    return toPolygon(shape);
}

EPolygonWithHolesData ERectangle::GetPolygonWithHoles() const
{
    EPolygonWithHolesData pwh;
    pwh.outline = GetContour();
    return pwh;
}

ERectangle::ERectangle(EBox2D box)
{
    std::swap(shape, box);
}

ERectangle::ERectangle(EPoint2D ll, EPoint2D ur)
{
    shape = EBox2D(std::move(ll), std::move(ur));
}

void ERectangle::Transform(const ETransform2D & trans)
{
    shape = Extent(trans.GetTransform() * shape);
}

EShapeType ERectangle::GetShapeType() const
{
    return EShapeType::Rectangle;
}

bool ERectangle::isValid() const
{
    return shape.isValid();
}

///EPath
bool EPath::hasHole() const
{
    return false;
}

EBox2D EPath::GetBBox() const
{
    return Extent(GetContour());
}

EPolygonData EPath::GetContour() const
{
    return toPolygon(shape, m_width);
}
    
EPolygonWithHolesData EPath::GetPolygonWithHoles() const
{
    EPolygonWithHolesData pwh;
    pwh.outline = GetContour();
    return pwh;
}

void EPath::Transform(const ETransform2D & trans)
{
    geometry::Transform(shape, trans.GetTransform());
}

EShapeType EPath::GetShapeType() const
{
    return EShapeType::Path;
}

bool EPath::isValid() const
{
    return !shape.empty() && math::NE<ECoord>(m_width, 0);
}

void EPath::SetPoints(const std::vector<EPoint2D> & points)
{
    shape = points;
}

void EPath::SetType(int type)
{
    m_type = type;
}

void EPath::SetWidth(ECoord width)
{
    m_width = width;
}
///ECircle
ECircle::ECircle(EPoint2D o, ECoord r, size_t div)
 : r(r), o(o), div(div)
{
}

bool ECircle::hasHole() const
{
    return false;
}

EBox2D ECircle::GetBBox() const
{
    EPoint2D offset(r, r);
    return EBox2D(o - offset, o + offset);
}

EPolygonData ECircle::GetContour() const
{
    return InscribedPolygon(Circle<ECoord>(o, r), div);
}

EPolygonWithHolesData ECircle::GetPolygonWithHoles() const
{
    EPolygonWithHolesData pwh;
    pwh.outline = GetContour();
    return pwh;
}

void ECircle::Transform(const ETransform2D & trans)
{
    geometry::Transform(o, trans.GetTransform());    
}

EShapeType ECircle::GetShapeType() const
{
    return EShapeType::Circle;
}

bool ECircle::isValid() const
{
    return math::GT<ECoord>(r, 0) && div >= 3;
}

void ECircle::PrintImp(std::ostream & os) const
{
    os << "CIRCLE(O:" << o[0] << ' ' << o[1] << ", R:" << r << ", DIV:" << div;
}

///EPolygon
EPolygon::EPolygon(std::vector<EPoint2D> points)
{
    shape.Set(std::move(points));
}

bool EPolygon::hasHole() const
{
    return false;
}

EBox2D EPolygon::GetBBox() const
{
    return Extent(shape);
}

EPolygonData EPolygon::GetContour() const
{
    return shape;
}
    
EPolygonWithHolesData EPolygon::GetPolygonWithHoles() const
{
    EPolygonWithHolesData pwh;
    pwh.outline = GetContour();
    return pwh;
}

void EPolygon::Transform(const ETransform2D & trans)
{
    geometry::Transform(shape, trans.GetTransform());    
}

EShapeType EPolygon::GetShapeType() const
{
    return EShapeType::Polygon;
}

bool EPolygon::isValid() const
{
    return shape.Size() >= 3;
}

void EPolygon::SetPoints(const std::vector<EPoint2D> & points)
{
    shape.Set(points);
}

EPolygon EPolygon::ConvexHull(const EPolygon & other)
{
    EPolygon polygon;
    std::vector<EPolygonData > shapes { shape, other.shape };
    auto convexHull = geometry::ConvexHull(shapes);
    polygon.shape = std::move(convexHull);
    return polygon;
}

///EPolygonWithHoles
bool EPolygonWithHoles::hasHole() const
{
    return shape.hasHole();
}

EBox2D EPolygonWithHoles::GetBBox() const
{
    return Extent(shape);
}

EPolygonData EPolygonWithHoles::GetContour() const
{
    return shape.outline;
}

EPolygonWithHolesData EPolygonWithHoles::GetPolygonWithHoles() const
{
    return shape;
}

void EPolygonWithHoles::Transform(const ETransform2D & trans)
{
    geometry::Transform(shape, trans.GetTransform());    
}

EShapeType EPolygonWithHoles::GetShapeType() const
{
    return EShapeType::PolygonWithHoles;
}

bool EPolygonWithHoles::isValid() const
{
    return shape.outline.Size() >= 3;
}

///EShapeFromTemplate
EShapeFromTemplate::EShapeFromTemplate(Template ts, ETransform2D trans)
 : m_transform(std::move(trans))
 , m_template(ts)
{
}

bool EShapeFromTemplate::hasHole() const
{
    if(!isValid()) return false;
    return m_template->hasHole();
}

EBox2D EShapeFromTemplate::GetBBox() const
{
    if(!isValid()) return EBox2D();
    auto box = m_template->GetBBox();
    return Extent(m_transform.GetTransform() * box);
}

EPolygonData EShapeFromTemplate::GetContour() const
{
    if(!isValid()) return EPolygonData();
    auto res = m_template->GetContour();
    geometry::Transform(res, m_transform.GetTransform()); 
    return res;   
}

EPolygonWithHolesData EShapeFromTemplate::GetPolygonWithHoles() const
{
    if(!isValid()) return EPolygonWithHolesData();
    auto res = m_template->GetPolygonWithHoles();
    geometry::Transform(res, m_transform.GetTransform()); 
    return res;
}

void EShapeFromTemplate::Transform(const ETransform2D & trans)
{
    m_transform.Append(trans);
}

EShapeType EShapeFromTemplate::GetShapeType() const
{
    return EShapeType::FromTemplate;
}

bool EShapeFromTemplate::isValid() const
{
    if(m_template) return m_template->isValid();
    return false;
}

}//namespace ecad