#pragma once
#include "interface/IComponentDefCollection.h"
#include "interface/IDefinitionCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
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

    Ptr<IDefinitionCollection> AddDefinitionCollection(EDefinitionType type) override;
    Ptr<IDefinitionCollection> GetDefinitionCollection(EDefinitionType type) const override;
    Ptr<IDefinition> AddDefinition(const std::string & name, UPtr<IDefinition> definition) override;
    Ptr<IDefinition> GetDefinition(const std::string & name, EDefinitionType type) const override;
    std::string GetNextDefName(const std::string & base, EDefinitionType type) const override;

    ComponentDefIter GetComponentDefIter() const override;
    size_t Size() const override;
    void Clear() override;

protected:
    ///Copy
    virtual Ptr<EComponentDefCollection> CloneImp() const override { return new EComponentDefCollection(*this); }
    virtual void PrintImp(std::ostream & os) const override;
};
}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentDefCollection)