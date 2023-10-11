#pragma once
#include "ECadCommon.h"
#include "EShape.h"

namespace ecad {

class ILayoutView;
namespace eutils {

ECAD_API UPtr<EPolygon> CalculateBoundary(CPtr<ILayoutView> layout);

}//nmmespace eutils
}//nmmespace ecad