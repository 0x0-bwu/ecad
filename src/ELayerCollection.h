#ifndef ECAD_ELAYERCOLLECTION_H
#define ECAD_ELAYERCOLLECTION_H
#include "interfaces/ILayerCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
#include "EUidGenerator.h"
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

    ELayerId AppendLayer(UPtr<ILayer> layer);
    std::vector<ELayerId> AppendLayers(std::vector<UPtr<ILayer> > layers);
    UPtr<ILayerMap> AddDefaultDielectricLayers();
    UPtr<ILayerMap> GetDefaultLayerMap() const;

    void GetStackupLayers(std::vector<Ptr<ILayer> > & layers) const;
    void GetStackupLayers(std::vector<CPtr<ILayer> > & layers) const;

    LayerIter GetLayerIter() const;
    size_t Size() const;

    Ptr<ILayer> FindLayerByLayerId(ELayerId lyrId) const;
    Ptr<ILayer> FindLayerByName(const std::string & name) const;
    std::string GetNextLayerName(const std::string & base) const;

protected:
    ///Copy
    Ptr<ELayerCollection> CloneImp() const override;
    UPtr<ILayerMap> MakeLayerIdOrdered();
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ELayerCollection)

#ifdef ECAD_HEADER_ONLY
#include "ELayerCollection.cpp"
#endif

#endif//ECAD_ELAYERCOLLECTION_H