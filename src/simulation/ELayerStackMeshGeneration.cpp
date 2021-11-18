#ifndef ECAD_HEADER_ONLY
#include "ELayerStackMeshGeneration.h"
#endif

#include "generic/geometry/TetrahedralizationIO.hpp"
#include "Interface.h"
namespace ecad {
namespace esim {

ECAD_INLINE ELayerStackMeshGeneration::ELayerStackMeshGeneration(CPtr<ILayoutView> layout)
 : m_layout(layout)
{
}

ECAD_INLINE ELayerStackMeshGeneration::~ELayerStackMeshGeneration()
{
}

ECAD_INLINE bool ELayerStackMeshGeneration::GenerateMesh()
{

    tet::PiecewiseLinearComplex<EPoint3D> plc;

    
    return false;
}
}//namespace esim
}//namespace ecad