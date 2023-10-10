#pragma once
#include "interfaces/IDefinitionCollection.h"
#include "interfaces/ILayerMapCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
namespace ecad {

class ILayerMap;
using ELayerMapIterator = EUnorderedMapCollectionIterator<std::string, UPtr<ILayerMap> >;
class ECAD_API ELayerMapCollection
 : public EUnorderedMapCollection<std::string, UPtr<ILayerMap> >
 , public IDefinitionCollection
 , public ILayerMapCollection
{
    using BaseCollection = EUnorderedMapCollection<std::string, UPtr<ILayerMap> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ELayerMapCollection();
    virtual ~ELayerMapCollection();

    ///Copy
    ELayerMapCollection(const ELayerMapCollection & other);
    ELayerMapCollection & operator= (const ELayerMapCollection & other);

    Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type);
    Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const;
    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition);
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const;

    LayerMapIter GetLayerMapIter() const;
    size_t Size() const;
    void Clear();

protected:
    ///Copy
    virtual Ptr<ELayerMapCollection> CloneImp() const override { return new ELayerMapCollection(*this); }
};
}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ELayerMapCollection)

#ifdef ECAD_HEADER_ONLY
#include "ELayerMapCollection.cpp"
#endif