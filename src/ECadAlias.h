#pragma once
#include "ECadConfig.h"
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

template <typename T, typename Deleter = std::default_delete<T> >
using UPtr = std::unique_ptr<T, Deleter>;

using EFloat = double;
using FCoord = double;
using ECoord = int64_t;

using EPoint2D = generic::geometry::Point2D<ECoord>;
using FPoint2D = generic::geometry::Point2D<FCoord>;
using EPoint3D = generic::geometry::Point3D<ECoord>;
using FPoint3D = generic::geometry::Point3D<FCoord>;

using EVector2D = EPoint2D;
using FVector2D = FPoint2D;
using EVector3D = EPoint3D;
using FVector3D = FPoint3D;

using EBox2D = generic::geometry::Box2D<ECoord>;
using FBox2D = generic::geometry::Box2D<FCoord>;
using EBox3D = generic::geometry::Box3D<ECoord>;
using FBox3D = generic::geometry::Box3D<FCoord>;

using EUuid = boost::uuids::uuid;

class EShape;
using ETemplateShape = SPtr<const EShape>;

ECAD_ALWAYS_INLINE static constexpr char sPadstack[] = "padstack";
ECAD_ALWAYS_INLINE static constexpr char sDefaultConductingMat[] = "copper";
ECAD_ALWAYS_INLINE static constexpr char sDefaultDielectricMat[] = "silicon";

ECAD_ALWAYS_INLINE static constexpr size_t invalidIndex = std::numeric_limits<size_t>::max();
ECAD_ALWAYS_INLINE static constexpr EFloat invalidFloat = std::numeric_limits<EFloat>::max();//std::nan(quiet_Nan) has issue with AppleClang with -ffast-math

template <typename NumType>
ECAD_ALWAYS_INLINE  bool isValid(NumType value) { return value != std::numeric_limits<NumType>::max(); }
}//namespace ecad