#pragma once
#include "ECadCommon.h"
#include "ETransform.h"
#include "IIterator.h"
#include "Protocol.h"
namespace ecad {

class ILayoutView;
class ECAD_API ICellInst : public Clonable<ICellInst>, public Transformable2D
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ICellInst() = default;
    virtual void SetRefLayoutView(CPtr<ILayoutView> refLayout) = 0;
    virtual CPtr<ILayoutView> GetRefLayoutView() const = 0;
    virtual void SetDefLayoutView(CPtr<ILayoutView> defLayout) = 0;
    virtual CPtr<ILayoutView> GetDefLayoutView() const = 0;
    virtual CPtr<ILayoutView> GetFlattenedLayoutView() const = 0;
};

}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ICellInst)