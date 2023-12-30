#pragma once
#include "ECadCommon.h"
#include "EShape.h"

namespace ecad {

class ILayoutView;
namespace utils {

ECAD_API UPtr<EPolygon> CalculateBoundary(CPtr<ILayoutView> layout);

}//nmmespace utils
}//nmmespace ecad