#pragma once
#include "ECadCommon.h"
#include "IIterator.h"
#include "Protocol.h"
namespace ecad {

class ICell;
class ECAD_API ICellCollection : public Clonable<ICellCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ICellCollection() = default;
    virtual CellIter GetCellIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ICellCollection)