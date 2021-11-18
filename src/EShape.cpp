#ifndef ECAD_HEADER_ONLY
#include "EShape.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPolygonWithHoles)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::ERectangle)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPolygon)
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPath)
#endif

#include "generic/geometry/Utility.hpp"
#include "ETransform.h"
namespace ecad {

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
    ar & boost::serialization::make_nvp("bbox", m_bbox);
}

template <typename Archive>
ECAD_INLINE void EPath::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPath, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("type", m_type);
    ar & boost::serialization::make_nvp("width", m_width);
    ar & boost::serialization::make_nvp("bbox", m_bbox);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPath)

template <typename Archive>
ECAD_INLINE void EPolygon::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygon, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("bbox", m_bbox);
}

template <typename Archive>
ECAD_INLINE void EPolygon::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygon, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("bbox", m_bbox);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPolygon)

template <typename Archive>
ECAD_INLINE void EPolygonWithHoles::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygonWithHoles, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("bbox", m_bbox);
}

template <typename Archive>
ECAD_INLINE void EPolygonWithHoles::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPolygonWithHoles, EShape>();
    ar & boost::serialization::make_nvp("shape", shape);
    ar & boost::serialization::make_nvp("bbox", m_bbox);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPolygonWithHoles)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE bool ERectangle::hasHole() const
{
    return false; 
}

ECAD_INLINE const EBox2D & ERectangle::GetBBox() const
{
    return shape;
}

ECAD_INLINE void ERectangle::SetBBox(EBox2D bbox)
{
    shape = std::move(bbox);
}

ECAD_INLINE Polygon2D<ECoord> ERectangle::GetContour() const
{
    return toPolygon(shape);
}

ECAD_INLINE PolygonWithHoles2D<ECoord> ERectangle::GetPolygonWithHoles() const
{
    PolygonWithHoles2D<ECoord> pwh;
    pwh.outline = std::move(GetContour());
    return pwh;
}

ECAD_INLINE void ERectangle::Transform(const ETransform2D & trans)
{
    shape = Extent(trans.GetTransform() * shape);
}

///EPath
ECAD_INLINE bool EPath::hasHole() const
{
    return false;
}

ECAD_INLINE const EBox2D & EPath::GetBBox() const
{
    if(!m_bbox.isValid())
        m_bbox = Extent(GetContour());
    return m_bbox;
}

ECAD_INLINE void EPath::SetBBox(EBox2D bbox)
{
    m_bbox = std::move(bbox);
}    

ECAD_INLINE Polygon2D<ECoord> EPath::GetContour() const
{
    return generic::geometry::toPolygon(shape, m_width);
}
    
ECAD_INLINE PolygonWithHoles2D<ECoord> EPath::GetPolygonWithHoles() const
{
    PolygonWithHoles2D<ECoord> pwh;
    pwh.outline = std::move(GetContour());
    return pwh;
}

ECAD_INLINE void EPath::Transform(const ETransform2D & trans)
{
    m_bbox.SetInvalid();
    generic::geometry::Transform(shape, trans.GetTransform());
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

///EPolygon
ECAD_INLINE bool EPolygon::hasHole() const
{
    return false;
}

ECAD_INLINE const EBox2D & EPolygon::GetBBox() const
{
    if(!m_bbox.isValid())
        m_bbox = Extent(shape);
    return m_bbox;
}

ECAD_INLINE void EPolygon::SetBBox(EBox2D bbox)
{
    m_bbox = std::move(bbox);
}

ECAD_INLINE Polygon2D<ECoord> EPolygon::GetContour() const
{
    return shape;
}
    
ECAD_INLINE PolygonWithHoles2D<ECoord> EPolygon::GetPolygonWithHoles() const
{
    PolygonWithHoles2D<ECoord> pwh;
    pwh.outline = std::move(GetContour());
    return pwh;
}

ECAD_INLINE void EPolygon::Transform(const ETransform2D & trans)
{
    m_bbox.SetInvalid();
    generic::geometry::Transform(shape, trans.GetTransform());    
}


ECAD_INLINE void EPolygon::SetPoints(const std::vector<EPoint2D> & points)
{
    shape.Set(points);
}

ECAD_INLINE EPolygon EPolygon::ConvexHull(const EPolygon & other)
{
    EPolygon polygon;
    std::vector<Polygon2D<ECoord> > shapes { shape, other.shape };
    auto convexHull = generic::geometry::ConvexHull(shapes);
    polygon.shape = std::move(convexHull);
    return polygon;
}

///EPolygonWithHoles
ECAD_INLINE bool EPolygonWithHoles::hasHole() const
{
    return shape.hasHole();
}

ECAD_INLINE const EBox2D & EPolygonWithHoles::GetBBox() const
{
    if(!m_bbox.isValid())
        m_bbox = Extent(shape);
    return m_bbox;
}

ECAD_INLINE void EPolygonWithHoles::SetBBox(EBox2D bbox)
{
    m_bbox = std::move(bbox);
}

ECAD_INLINE Polygon2D<ECoord> EPolygonWithHoles::GetContour() const
{
    return shape.outline;
}

ECAD_INLINE PolygonWithHoles2D<ECoord> EPolygonWithHoles::GetPolygonWithHoles() const
{
    return shape;
}

ECAD_INLINE void EPolygonWithHoles::Transform(const ETransform2D & trans)
{
    m_bbox.SetInvalid();
    generic::geometry::Transform(shape, trans.GetTransform());    
}

}//namespace ecad