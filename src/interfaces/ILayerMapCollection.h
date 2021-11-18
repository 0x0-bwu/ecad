#ifndef ECAD_ILAYERMAPCOLLECTION_H
#define ECAD_ILAYERMAPCOLLECTION_H
#include "ECadCommon.h"
#include "IIterator.h"
#include "Protocol.h"
namespace ecad {
class ILayerMap;
class ECAD_API ILayerMapCollection : public Clonable<ILayerMapCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ILayerMapCollection() = default;
    virtual LayerMapIter GetLayerMapIter() const = 0;
    virtual size_t Size() const = 0;
    virtual void Clear() = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ILayerMapCollection)
#endif//ECAD_ILAYERCOLLECTION_H