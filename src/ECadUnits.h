#ifndef ECAD_ECADUNITS_H
#define ECAD_ECADUNITS_H
#include <boost/core/ignore_unused.hpp>
#include "generic/tools/Units.hpp"
#include "ECadSerialization.h"
#include "ECadAlias.h"
#define ECAD_UNUSED(ex) boost::ignore_unused(ex);

namespace ecad {

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