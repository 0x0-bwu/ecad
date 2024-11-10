#pragma once
#include "interface/ILayerCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
namespace ecad {

class ILayerMap;
using ELayerIterator = EVectorCollectionIterator<UPtr<ILayer> >;
class ECAD_API ELayerCollection : public EVectorCollection<UPtr<ILayer> >, public ILayerCollection
{
    using BaseCollection = EVectorCollection<UPtr<ILayer> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ELayerCollection();
    virtual ~ELayerCollection();

    ///Copy
    ELayerCollection(const ELayerCollection & other);
    ELayerCollection & operator= (const ELayerCollection & other);

    ELayerId AppendLayer(UPtr<ILayer> layer) override;
    std::vector<ELayerId> AppendLayers(std::vector<UPtr<ILayer> > layers) override;
    UPtr<ILayerMap> AddDefaultDielectricLayers() override;
    UPtr<ILayerMap> GetDefaultLayerMap() const override;

    void GetStackupLayers(std::vector<Ptr<IStackupLayer> > & layers) const override;
    void GetStackupLayers(std::vector<CPtr<IStackupLayer> > & layers) const override;

    LayerIter GetLayerIter() const override;
    size_t Size() const override;

    Ptr<ILayer> FindLayerByLayerId(ELayerId lyrId) const override;
    Ptr<ILayer> FindLayerByName(const std::string & name) const override;
    std::string GetNextLayerName(const std::string & base) const;

protected:
    UPtr<ILayerMap> MakeLayerIdOrdered();

protected:
    virtual ECollectionType GetType() const override { return ECollectionType::Layer; }
    virtual Ptr<ELayerCollection> CloneImp() const override;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ELayerCollection)