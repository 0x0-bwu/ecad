#ifndef ECAD_ILAYERCOLLECTION_H
#define ECAD_ILAYERCOLLECTION_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include "IIterator.h"
#include "Protocol.h"
namespace ecad {
class ILayer;
class ILayerMap;
class ECAD_API ILayerCollection : public Clonable<ILayerCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ILayerCollection() = default;
    virtual ELayerId AppendLayer(UPtr<ILayer> layer) = 0;
    virtual std::vector<ELayerId> AppendLayers(std::vector<UPtr<ILayer> > layers) = 0;
    virtual UPtr<ILayerMap> AddDefaultDielectricLayers() = 0;
    virtual UPtr<ILayerMap> GetDefaultLayerMap() const = 0;
    virtual void GetStackupLayers(std::vector<Ptr<ILayer> > & layers) const = 0;
    virtual void GetStackupLayers(std::vector<CPtr<ILayer> > & layers) const = 0;
    virtual LayerIter GetLayerIter() const = 0;
    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ILayerCollection)
#endif//ECAD_ILAYERCOLLECTION_H