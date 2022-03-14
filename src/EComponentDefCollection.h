#ifndef ECAD_ECOMPONENTDEFCOLLECTION_H
#define ECAD_ECOMPONENTDEFCOLLECTION_H
#include "interfaces/IComponentDefCollection.h"
#include "interfaces/IDefinitionCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
namespace ecad {

class IComponentDef;
using EComponentDefIterator = EUnorderedMapCollectionIterator<std::string, UPtr<IComponentDef> >;
class ECAD_API EComponentDefCollection
 : public EUnorderedMapCollection<std::string, UPtr<IComponentDef> >
 , public IComponentDefCollection
 , public IDefinitionCollection
{
    using BaseCollection = EUnorderedMapCollection<std::string, UPtr<IComponentDef> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EComponentDefCollection();
    virtual ~EComponentDefCollection();

    ///Copy
    EComponentDefCollection(const EComponentDefCollection & other);
    EComponentDefCollection & operator= (const EComponentDefCollection & other);

    Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type);
    Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const;
    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition);
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const;

    ComponentDefIter GetComponentDefIter() const;
    size_t Size() const;
    void Clear();

protected:
    ///Copy
    virtual Ptr<EComponentDefCollection> CloneImp() const override { return new EComponentDefCollection(*this); }
};
}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentDefCollection)

#ifdef ECAD_HEADER_ONLY
#include "EComponentDefCollection.cpp"
#endif

#endif//ECAD_ECOMPONENTDEFCOLLECTION_H