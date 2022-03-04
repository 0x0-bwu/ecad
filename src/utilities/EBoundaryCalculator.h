#ifndef ECAD_EUIT_EBOUNDARYCALCULATOR_H
#define ECAD_EUIT_EBOUNDARYCALCULATOR_H
#include "ECadCommon.h"
#include "EShape.h"

namespace ecad {

class ILayoutView;
namespace euti {

ECAD_API UPtr<EPolygon> CalculateBoundary(CPtr<ILayoutView> layout);

}//nmmespace euti
}//nmmespace ecad

#ifdef ECAD_HEADER_ONLY
#include "utilities/EBoundaryCalculator.cpp"
#endif//ECAD_HEADER_ONLY

#endif//ECAD_EUIT_EBOUNDARYCALCULATOR_H