#ifndef ECAD_ECADALIAS_H
#define ECAD_ECADALIAS_H
#include "generic/geometry/Box.hpp"
#include <boost/uuid/uuid.hpp>
#include <cstddef>
#include <memory>
namespace ecad {

using std::size_t;

template <typename T>
using Ptr = T*;

template <typename T>
using CPtr = const T*;

template <typename T>
using SPtr = std::shared_ptr<T>;

template <typename T>
using UPtr = std::unique_ptr<T>;

using ECoord = int64_t;
using FCoord = double;
using EValue = double;

using EPoint2D = generic::geometry::Point2D<ECoord>;
using FPoint2D = generic::geometry::Point2D<FCoord>;
using EPoint3D = generic::geometry::Point3D<ECoord>;
using FPoint3D = generic::geometry::Point3D<FCoord>;

using EBox2D = generic::geometry::Box2D<ECoord>;
using FBox2D = generic::geometry::Box2D<FCoord>;
using EBox3D = generic::geometry::Box3D<ECoord>;
using FBox3D = generic::geometry::Box3D<FCoord>;

using EUuid = boost::uuids::uuid;

ECAD_ALWAYS_INLINE static constexpr char sPadstack[] = "padstack";
ECAD_ALWAYS_INLINE static constexpr char sDefaultConductingMat[] = "copper";
ECAD_ALWAYS_INLINE static constexpr char sDefaultDielectricMat[] = "silicon";

ECAD_ALWAYS_INLINE static constexpr size_t invalidIndex = std::numeric_limits<size_t>::max();
}//namespace ecad
#endif//ECAD_ECADALIAS_H