#pragma once
#include "ECadDef.h"
#include "generic/geometry/Triangle.hpp"
#include "generic/geometry/Box.hpp"
#include "generic/tools/Log.hpp"
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

template <typename T1, typename T2>
using EPair = std::pair<T1, T2>;

using Int32 = int32_t;
using Int64 = int64_t;
using Float32 = float;
using Float64 = double;
using EFloat = Float64;
using FCoord = EFloat;
using ECoord = Int64;
using ESize = size_t;
using EIndex = ESize;



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

using ETriangle2D = generic::geometry::Triangle2D<ECoord>;
using FTriangle2D = generic::geometry::Triangle2D<FCoord>;

using EUuid = boost::uuids::uuid;
using ENetIdSet = std::unordered_set<ENetId>;
using ELayerIdSet = std::unordered_set<ELayerId>;

class EShape;
using ETemplateShape = SPtr<const EShape>;

using ELogLevel = generic::log::Level;

using ETemperatureUnit = generic::unit::Temperature;

using EScenarioId = ESize;

ECAD_ALWAYS_INLINE static constexpr char sPadstack[] = "padstack";
ECAD_ALWAYS_INLINE static constexpr char sDefaultConductingMat[] = "copper";
ECAD_ALWAYS_INLINE static constexpr char sDefaultDielectricMat[] = "silicon";

ECAD_ALWAYS_INLINE static constexpr ESize invalidIndex = std::numeric_limits<ESize>::max();
ECAD_ALWAYS_INLINE static constexpr EFloat invalidFloat = std::numeric_limits<EFloat>::max();//std::nan(quiet_Nan) has issue with AppleClang with -ffast-math
ECAD_ALWAYS_INLINE static constexpr EFloat maxFloat = std::numeric_limits<EFloat>::max();

template <typename NumType>
ECAD_ALWAYS_INLINE  bool isValid(NumType value) { return value != std::numeric_limits<NumType>::max(); }
}//namespace ecad

#define ECAD_TRACE(args...) generic::log::Trace(args)
#define ECAD_DEBUG(args...) generic::log::Debug(args)
#define ECAD_INFO(args...) generic::log::Info(args)
#define ECAD_ERROR(args...) generic::log::Error(args)