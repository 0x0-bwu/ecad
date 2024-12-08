#pragma once
#include "ECadConfig.h"
#include "generic/utils/Version.hpp"
namespace ecad {

using EVersion = generic::utils::Version;
ECAD_ALWAYS_INLINE static constexpr EVersion CURRENT_VERSION = EVersion(0, 0, 0);

/// Version history
//  0.0.0 : Beta.
//  1.0.0 : First release version.

}//namespace ecad