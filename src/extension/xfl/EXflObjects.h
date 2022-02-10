#ifndef ECAD_EXT_XFL_EXFLOBJECTS_H
#define ECAD_EXT_XFL_EXFLOBJECTS_H
#include "generic/geometry/Geometries.hpp"
#include "ECadCommon.h"
#include "EShape.h"
#include <unordered_map>
#include <unordered_set>
namespace ecad {

namespace ext {
namespace xfl {

using namespace generic::geometry;

enum class EXflUnit { INCH, MM };
enum class EXflLayerType { SIGNAL, DIELECTRIC, POWERGROUND };
using EXflVersion = std::pair<int, int>;//<major, minor>

struct EXflObject
{
    using LayerId = int16_t;
    virtual ~EXflObject() = default;
};

struct EXflMaterial : public EXflObject
{
    std::string name;
    bool isMetal;
    double conductivity;
    double permittivity;
    double permeability;
    double lossTangent;
    int causality;//0-none, 1-wideband debye, 2-multipole debye
    ~EXflMaterial() = default;
};

struct EXflLayer : public EXflObject
{
    std::string name;
    double thickness;
    EXflLayerType type;
    std::string conductingMatName;
    std::string dielectricMatName;
    ~EXflLayer() = default;
};

struct EXflShape : public EXflObject
{
    using BaseType = EXflObject;
    virtual ~EXflShape() = default;
};

struct EXflRectangle : public EXflShape
{
    FCoord width, height;
    ~EXflRectangle() = default;
};

struct EXflPolygon : public EXflShape
{
};

struct EXflDB : public EXflObject
{
    EXflVersion version;
    EXflUnit unit;
    FCoord scale;
    std::vector<EXflMaterial> materials;
    std::vector<EXflLayer> layers;
};

}//namespace xfl   
}//namespace ext
}//namespace ecad
#endif//ECAD_EXT_XFL_EXFLOBJECTS_H