#ifndef ECAD_ESIM_ELAYERSTACKMESHGENERATION_H
#define ECAD_ESIM_ELAYERSTACKMESHGENERATION_H
#include "ECadCommon.h"
namespace ecad {

class ILayoutView;
namespace esim {

using namespace generic::geometry;

class ECAD_API ELayerStackMeshGeneration
{
public:
    explicit ELayerStackMeshGeneration(CPtr<ILayoutView> layout);
    virtual ~ELayerStackMeshGeneration();
    bool GenerateMesh();

private:
    CPtr<ILayoutView> m_layout = nullptr;
};

}//namespace esim
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "ELayerStackMeshGeneration.cpp"
#endif

#endif//ECAD_ESIM_ELAYERSTACKMESHGENERATION_H
