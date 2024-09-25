#pragma once
#include "interface/IDefinitionCollection.h"
#include "interface/ILayerMapCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
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

    Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type) override;
    Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const override;
    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) override;
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const override;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const override;

    void SetDatabase(CPtr<IDatabase> database) override;
    LayerMapIter GetLayerMapIter() const override;
    size_t Size() const override;
    void Clear() override;

protected:
    ///Copy
    virtual Ptr<ELayerMapCollection> CloneImp() const override { return new ELayerMapCollection(*this); }
    virtual void PrintImp(std::ostream & os) const override;
};
}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ELayerMapCollection)