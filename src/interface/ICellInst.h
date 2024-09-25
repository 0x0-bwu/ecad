#pragma once
#include "basic/ECadCommon.h"
#include "IIterator.h"
namespace ecad {

class ILayerMap;
class ILayoutView;
class ECAD_API ICellInst : public Clonable<ICellInst>, public Transformable2D
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ICellInst() = default;
    virtual void SetName(std::string name) = 0;
    virtual const std::string & GetName() const = 0;
    virtual void SetRefLayoutView(CPtr<ILayoutView> refLayout) = 0;
    virtual CPtr<ILayoutView> GetRefLayoutView() const = 0;
    virtual void SetDefLayoutView(CPtr<ILayoutView> defLayout) = 0;
    virtual CPtr<ILayoutView> GetDefLayoutView() const = 0;
    virtual CPtr<ILayoutView> GetFlattenedLayoutView() const = 0;
    virtual void SetLayerMap(CPtr<ILayerMap> layerMap) = 0;
    virtual CPtr<ILayerMap> GetLayerMap() const = 0;
};

}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ICellInst)