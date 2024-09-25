#pragma once
#include "ECadConfig.h"
#include <utility>
#include <string>
namespace ecad {

using EVersion = std::pair<unsigned int, unsigned int>;//[Major(01), Minor(00)]
ECAD_ALWAYS_INLINE static constexpr EVersion CURRENT_VERSION = EVersion{0, 0};

ECAD_ALWAYS_INLINE unsigned int toInt(const EVersion & ver)
{
    return (ver.first % 100) * 100 + ver.second % 100;
}

ECAD_ALWAYS_INLINE EVersion fromInt(const unsigned int ver)
{
    EVersion version;
    version.first  = ver / 100 % 100;
    version.second = ver % 100;
    return version;
}

ECAD_ALWAYS_INLINE std::string toString(const EVersion & ver)
{
    return std::to_string(ver.first % 100) + "." + std::to_string(ver.second % 100);
}

ECAD_ALWAYS_INLINE bool VersionGreaterThan(const EVersion & ver1, const EVersion & ver2)
{
    return toInt(ver1) > toInt(ver2);
}

ECAD_ALWAYS_INLINE bool VersionLessThan(const EVersion & ver1, const EVersion & ver2)
{
    return toInt(ver1) < toInt(ver2);
}

/// Version history
//  0.0 : Beta.
//  1.0 : First release version.

}//namespace ecad