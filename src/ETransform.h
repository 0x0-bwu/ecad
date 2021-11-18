#ifndef ECAD_ETRANSFORM_H
#define ECAD_ETRANSFORM_H
#include "generic/geometry/Transform.hpp"
#include "generic/math/MathUtility.hpp"
#include "ECadCommon.h"
#include <list>
namespace ecad {

struct ECAD_API ETransformData2D
{
    using Transform = ::generic::geometry::Transform2D<EValue>;
    //Transform done in following order.
    EValue scale = 1.0;
    EValue rotation = 0;//Rotation (in radians) about point (0,0) in CCW direction
    bool   mirror = false;//Mirror about Y axis
    EPoint2D offset = EPoint2D(0, 0);

    bool isScaled() const { return math::NE<EValue>(scale, 1); }
    bool isRotated() const { return math::NE<EValue>(rotation, 0); }
    bool isMirrored() const { return mirror; }
    bool isOffseted() const { return offset != EPoint2D(0, 0); }
    bool isTransformed() const { return isScaled() || isRotated() || isMirrored() || isOffseted(); }

    Transform GetTransform() const
    {
        using namespace ::generic::geometry;
        Transform transfrom;
        if(isScaled()) transfrom = makeScaleTransform2D<EValue>(scale) * transfrom;
        if(isRotated()) transfrom = makeRotateTransform2D<EValue>(rotation) * transfrom;
        if(isMirrored()) transfrom = makeMirroredTransform2D<EValue>(Axis::Y) * transfrom;
        if(isOffseted()) transfrom = makeShiftTransform2D<EValue>(offset) * transfrom;
        return transfrom;
    }
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("scale", scale);
        ar & boost::serialization::make_nvp("rotation", rotation);
        ar & boost::serialization::make_nvp("mirror", mirror);
        ar & boost::serialization::make_nvp("offset", offset);
    }
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
};

class ECAD_API ETransform2D
{
    using Transform = ETransformData2D::Transform;

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("sequence", m_sequence);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        m_sequence.clear();
        ar & boost::serialization::make_nvp("sequence", m_sequence);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
public:
    ETransform2D() { m_sequence.push_back(ETransformData2D{}); }
    
    ~ETransform2D() = default;

    ///Copy
    ETransform2D(const ETransform2D & other) { *this = other; }

    ETransform2D & operator= (const ETransform2D & other)
    {
        m_transform.reset();
        m_sequence = other.m_sequence;
        if(other.m_transform)
            m_transform.reset(new Transform(*other.m_transform));  
        return *this;
    }

    EValue & Scale() { m_transform.reset(); return m_sequence.back().scale; }
    const EValue & Scale() const { return m_sequence.back().scale; }

    EValue & Rotation() { m_transform.reset(); return m_sequence.back().rotation; }
    const EValue & Rotation() const { return m_sequence.back().rotation; }

    bool & Mirror() { m_transform.reset(); return m_sequence.back().mirror; }
    const bool & Mirror() const { return m_sequence.back().mirror; }

    EPoint2D & Offset() { m_transform.reset(); return m_sequence.back().offset; }
    const EPoint2D & Offset() const { return m_sequence.back().offset; }

    void Append(const ETransform2D & transform)
    {
        if(m_transform) *m_transform = (*m_transform) * transform.GetTransform();
        for(const auto & sequence : transform.m_sequence){
            if(sequence.isTransformed())
                m_sequence.push_back(sequence);
        }
    }

    const Transform & GetTransform() const
    {
        using namespace ::generic::geometry;
        if(nullptr == m_transform){
            Transform transform;
            for(const auto & sequence : m_sequence){
                if(!sequence.isTransformed()) continue;
                transform = sequence.GetTransform() * transform;
            }
            m_transform.reset(new Transform(std::move(transform)));
        }
        return *m_transform;
    }
private:
    mutable UPtr<Transform> m_transform = nullptr;
    std::list<ETransformData2D> m_sequence;//FIFO
};

ECAD_ALWAYS_INLINE ETransform2D makeETransform2D(EValue scale, EValue rotation, const EPoint2D & offset, bool mirror = false)
{
    ETransform2D transform;
    transform.Scale() = scale;
    transform.Rotation() = rotation;
    transform.Offset() = offset;
    transform.Mirror() = mirror;
    return transform;
}

}//namespace ecad

#endif//ECAD_ETRANSFORM_H