#ifndef ECAD_EUTILS_EBOUNDARYCALCULATOR_H
#define ECAD_EUTILS_EBOUNDARYCALCULATOR_H
#include "ECadCommon.h"
#include "EShape.h"

namespace ecad {

class ILayoutView;
namespace eutils {

ECAD_API UPtr<EPolygon> CalculateBoundary(CPtr<ILayoutView> layout);

}//nmmespace eutils
}//nmmespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utilities/EBoundaryCalculator.cpp"
#endif//ECAD_HEADER_ONLY

#endif//ECAD_EUTILS_EBOUNDARYCALCULATOR_H