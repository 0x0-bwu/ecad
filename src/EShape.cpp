#include "EShape.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPolygonWithHoles)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ERectangle)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPolygon)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPath)

#include "generic/geometry/Utility.hpp"
#include "ETransform.h"
namespace ecad {

using namespace generic;
using namespace generic::geometry;

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void ERectangle::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ERectangle, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

template <typename Archive>
ECAD_INLINE void ERectangle::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<ERectangle, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ERectangle)

template <typename Archive>
ECAD_INLINE void EPath::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPath, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("width", m_width);
}

template <typename Archive>
ECAD_INLINE void EPath::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPath, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("width", m_width);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPath)

template <typename Archive>
ECAD_INLINE void ECircle::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygon, EShape>();
    ar & boost::serialization::make_nvp("o", o);
    ar & boost::serialization::make_nvp("r", r);
    ar & boost::serialization::make_nvp("div", div);
}

template <typename Archive>
ECAD_INLINE void ECircle::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygon, EShape>();
    ar & boost::serialization::make_nvp("o", o);
    ar & boost::serialization::make_nvp("r", r);
    ar & boost::serialization::make_nvp("div", div);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(ECircle)

template <typename Archive>
ECAD_INLINE void EPolygon::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygon, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

template <typename Archive>
ECAD_INLINE void EPolygon::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygon, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPolygon)

template <typename Archive>
ECAD_INLINE void EPolygonWithHoles::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygonWithHoles, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

template <typename Archive>
ECAD_INLINE void EPolygonWithHoles::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygonWithHoles, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPolygonWithHoles)

template <typename Archive>
ECAD_INLINE void EShapeFromTemplate::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EShapeFromTemplate, EShape>();
    ar & boost::serialization::make_nvp("template", m_template);
    ar & boost::serialization::make_nvp("transform", m_transform);
}

template <typename Archive>
ECAD_INLINE void EShapeFromTemplate::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EShapeFromTemplate, EShape>();
    ar & boost::serialization::make_nvp("template", m_template);
    ar & boost::serialization::make_nvp("transform", m_transform);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EShapeFromTemplate)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE bool ERectangle::hasHole() const
{
    return false; 
}

ECAD_INLINE EBox2D ERectangle::GetBBox() const
{
    return shape;
}

ECAD_INLINE EPolygonData ERectangle::GetContour() const
{
    return toPolygon(shape);
}

ECAD_INLINE EPolygonWithHolesData ERectangle::GetPolygonWithHoles() const
{
    EPolygonWithHolesData pwh;
    pwh.outline = GetContour();
    return pwh;
}

ECAD_INLINE ERectangle::ERectangle(EPoint2D ll, EPoint2D ur)
{
    shape = EBox2D(std::move(ll), std::move(ur));
}

ECAD_INLINE void ERectangle::Transform(const ETransform2D & trans)
{
    shape = Extent(trans.GetTransform() * shape);
}

ECAD_INLINE EShapeType ERectangle::GetShapeType() const
{
    return EShapeType::Rectangle;
}

ECAD_INLINE bool ERectangle::isValid() const
{
    return shape.isValid();
}

///EPath
ECAD_INLINE bool EPath::hasHole() const
{
    return false;
}

ECAD_INLINE EBox2D EPath::GetBBox() const
{
    return Extent(GetContour());
}

ECAD_INLINE EPolygonData EPath::GetContour() const
{
    return toPolygon(shape, m_width);
}
    
ECAD_INLINE EPolygonWithHolesData EPath::GetPolygonWithHoles() const
{
    EPolygonWithHolesData pwh;
    pwh.outline = GetContour();
    return pwh;
}

ECAD_INLINE void EPath::Transform(const ETransform2D & trans)
{
    geometry::Transform(shape, trans.GetTransform());
}

ECAD_INLINE EShapeType EPath::GetShapeType() const
{
    return EShapeType::Path;
}

ECAD_INLINE bool EPath::isValid() const
{
    return !shape.empty() && math::NE<ECoord>(m_width, 0);
}

ECAD_INLINE void EPath::SetPoints(const std::vector<EPoint2D> & points)
{
    shape = points;
}

ECAD_INLINE void EPath::SetType(int type)
{
    m_type = type;
}

ECAD_INLINE void EPath::SetWidth(ECoord width)
{
    m_width = width;
}
///ECircle
ECAD_INLINE ECircle::ECircle(EPoint2D o, ECoord r, size_t div)
 : r(r), o(o), div(div)
{
}

ECAD_INLINE bool ECircle::hasHole() const
{
    return false;
}

ECAD_INLINE EBox2D ECircle::GetBBox() const
{
    EPoint2D offset(r, r);
    return EBox2D(o - offset, o + offset);
}

ECAD_INLINE EPolygonData ECircle::GetContour() const
{
    return InscribedPolygon(Circle<ECoord>(o, r), div);
}

ECAD_INLINE EPolygonWithHolesData ECircle::GetPolygonWithHoles() const
{
    EPolygonWithHolesData pwh;
    pwh.outline = GetContour();
    return pwh;
}

ECAD_INLINE void ECircle::Transform(const ETransform2D & trans)
{
    geometry::Transform(o, trans.GetTransform());    
}

ECAD_INLINE EShapeType ECircle::GetShapeType() const
{
    return EShapeType::Circle;
}

ECAD_INLINE bool ECircle::isValid() const
{
    return math::GT<ECoord>(r, 0) && div >= 3;
}

ECAD_INLINE void ECircle::PrintImp(std::ostream & os) const
{
    os << "CIRCLE(O:" << o[0] << ' ' << o[1] << ", R:" << r << ", DIV:" << div;
}

///EPolygon
ECAD_INLINE EPolygon::EPolygon(std::vector<EPoint2D> points)
{
    shape.Set(std::move(points));
}

ECAD_INLINE bool EPolygon::hasHole() const
{
    return false;
}

ECAD_INLINE EBox2D EPolygon::GetBBox() const
{
    return Extent(shape);
}

ECAD_INLINE EPolygonData EPolygon::GetContour() const
{
    return shape;
}
    
ECAD_INLINE EPolygonWithHolesData EPolygon::GetPolygonWithHoles() const
{
    EPolygonWithHolesData pwh;
    pwh.outline = GetContour();
    return pwh;
}

ECAD_INLINE void EPolygon::Transform(const ETransform2D & trans)
{
    geometry::Transform(shape, trans.GetTransform());    
}

ECAD_INLINE EShapeType EPolygon::GetShapeType() const
{
    return EShapeType::Polygon;
}

ECAD_INLINE bool EPolygon::isValid() const
{
    return shape.Size() >= 3;
}

ECAD_INLINE void EPolygon::SetPoints(const std::vector<EPoint2D> & points)
{
    shape.Set(points);
}

ECAD_INLINE EPolygon EPolygon::ConvexHull(const EPolygon & other)
{
    EPolygon polygon;
    std::vector<EPolygonData > shapes { shape, other.shape };
    auto convexHull = geometry::ConvexHull(shapes);
    polygon.shape = std::move(convexHull);
    return polygon;
}

///EPolygonWithHoles
ECAD_INLINE bool EPolygonWithHoles::hasHole() const
{
    return shape.hasHole();
}

ECAD_INLINE EBox2D EPolygonWithHoles::GetBBox() const
{
    return Extent(shape);
}

ECAD_INLINE EPolygonData EPolygonWithHoles::GetContour() const
{
    return shape.outline;
}

ECAD_INLINE EPolygonWithHolesData EPolygonWithHoles::GetPolygonWithHoles() const
{
    return shape;
}

ECAD_INLINE void EPolygonWithHoles::Transform(const ETransform2D & trans)
{
    geometry::Transform(shape, trans.GetTransform());    
}

ECAD_INLINE EShapeType EPolygonWithHoles::GetShapeType() const
{
    return EShapeType::PolygonWithHoles;
}

ECAD_INLINE bool EPolygonWithHoles::isValid() const
{
    return shape.outline.Size() >= 3;
}

///EShapeFromTemplate
ECAD_INLINE EShapeFromTemplate::EShapeFromTemplate(Template ts, ETransform2D trans)
 : m_transform(std::move(trans))
 , m_template(ts)
{
}

ECAD_INLINE bool EShapeFromTemplate::hasHole() const
{
    if(!isValid()) return false;
    return m_template->hasHole();
}

ECAD_INLINE EBox2D EShapeFromTemplate::GetBBox() const
{
    if(!isValid()) return EBox2D();
    auto box = m_template->GetBBox();
    return Extent(m_transform.GetTransform() * box);
}

ECAD_INLINE EPolygonData EShapeFromTemplate::GetContour() const
{
    if(!isValid()) return EPolygonData();
    auto res = m_template->GetContour();
    geometry::Transform(res, m_transform.GetTransform()); 
    return res;   
}

ECAD_INLINE EPolygonWithHolesData EShapeFromTemplate::GetPolygonWithHoles() const
{
    if(!isValid()) return EPolygonWithHolesData();
    auto res = m_template->GetPolygonWithHoles();
    geometry::Transform(res, m_transform.GetTransform()); 
    return res;
}

ECAD_INLINE void EShapeFromTemplate::Transform(const ETransform2D & trans)
{
    m_transform.Append(trans);
}

ECAD_INLINE EShapeType EShapeFromTemplate::GetShapeType() const
{
    return EShapeType::FromTemplate;
}

ECAD_INLINE bool EShapeFromTemplate::isValid() const
{
    if(m_template) return m_template->isValid();
    return false;
}

}//namespace ecad