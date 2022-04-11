#ifndef ECAD_ECADUNITS_H
#define ECAD_ECADUNITS_H
#include <boost/core/ignore_unused.hpp>
#include "generic/tools/Units.hpp"
#include "ECadSerialization.h"
#include "ECadAlias.h"
#define ECAD_UNUSED(ex) boost::ignore_unused(ex);

namespace ecad {

struct ESize2D
{
    size_t x = 0, y = 0;
    ESize2D() = default;
    ESize2D(size_t x, size_t y) : x(x), y(y) {}

    size_t & operator[] (size_t i)
    {
        if(0 == i) return x;
        else return y; 
    }

    const size_t & operator[] (size_t i) const
    {
        if(0 == i) return x;
        else return y; 
    }

    bool operator== (const ESize2D & other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!= (const ESize2D & other) const
    {
        return !(*this == other);
    }

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("x", x);
        ar & boost::serialization::make_nvp("y", y);
    }

    template <typename Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("x", x);
        ar & boost::serialization::make_nvp("y", y);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
};

struct ESize3D
{
    size_t x = 0, y = 0, z = 0;
    ESize3D() = default;
    ESize3D(size_t x, size_t y, size_t z) : x(x), y(y), z(z) {}
    ESize3D(const ESize2D & size, size_t z) : x(size.x), y(size.y), z(z) {}

    size_t & operator[] (size_t i)
    {
        if(0 == i) return x;
        else if(1 == i) return y;
        else return z;
    }

    const size_t & operator[] (size_t i) const
    {
        if(0 == i) return x;
        else if(1 == i) return y;
        else return z;
    }

    bool operator== (const ESize3D & other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!= (const ESize3D & other) const
    {
        return !(*this == other);
    }

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("x", x);
        ar & boost::serialization::make_nvp("y", y);
        ar & boost::serialization::make_nvp("z", z);
    }

    template <typename Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("x", x);
        ar & boost::serialization::make_nvp("y", y);
        ar & boost::serialization::make_nvp("z", z);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
};

class ECoordUnits
{
    EValue unit = 1e-3;
    EValue precision = 1e-9;
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    template <typename Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("unit", unit);
        ar & boost::serialization::make_nvp("precision", precision);
    }

    template <typename Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("unit", unit);
        ar & boost::serialization::make_nvp("precision", precision);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
public:
    using Unit = generic::unit::Length;
    /**
     * @brief Construct a new ECoordUnits object
     * 
     * @param user user unit, defualt is um
     * @param data database unit, default is nm
     */
    ECoordUnits(Unit user = Unit::Micrometer, Unit data = Unit::Nanometer)
    {
        precision = generic::unit::Scale2Meter(data);
        unit = precision / generic::unit::Scale2Meter(user);
    }
    
    void SetPrecision(EValue value)
    {
        precision = value;
    }

    void SetUnit(EValue value)
    {
        unit = value;
    }
    
    EValue Scale2Unit() const
    {
        return unit;
    }

    EValue Scale2Coord() const
    {
        return EValue(1) / unit;
    }

    ECoord toCoord(const EValue value) const
    {
        return value * Scale2Coord();
    }

    ECoord toCoord(const EValue value, Unit unit) const
    {
        return value * generic::unit::Scale2Meter(unit) / precision;
    }

    FCoord toCoordF(const EValue value) const
    {
        return value * Scale2Coord();
    }

    FCoord toCoordF(const EValue value, Unit unit) const
    {
        return value * generic::unit::Scale2Meter(unit) / precision;
    }
    
    template <typename Coord>
    EValue toUnit(const Coord coord) const
    {
        return coord * Scale2Unit();
    }

    template <typename Coord>
    EValue toUnit(const Coord coord, Unit unit) const
    {
        return coord * precision / generic::unit::Scale2Meter(unit);
    }
};

}//namespace ecad
#endif//ECAD_ECADUNITS_H