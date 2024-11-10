#pragma once
#include "interface/IComponentCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
namespace ecad {

class IComponent;
using EComponentIterator = EUnorderedMapCollectionIterator<std::string, UPtr<IComponent> >;
class ECAD_API EComponentCollection
 : public EUnorderedMapCollection<std::string, UPtr<IComponent> >
 , public IComponentCollection
{
    using BaseCollection = EUnorderedMapCollection<std::string, UPtr<IComponent> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EComponentCollection();
    virtual ~EComponentCollection();

    ///Copy
    EComponentCollection(const EComponentCollection & other);
    EComponentCollection & operator= (const EComponentCollection & other);

    Ptr<IComponent> AddComponent(UPtr<IComponent> component) override;

    Ptr<IComponent> CreateComponent(const std::string & name, CPtr<ILayoutView> refLayout, CPtr<IComponentDef> compDef, ELayerId layer, const ETransform2D & transform, bool flipped) override;

    ComponentIter GetComponentIter() const override;

    Ptr<IComponent> FindComponentByName(const std::string & name) const override;

    size_t Size() const override;
    void Clear() override;

protected:
    virtual ECollectionType GetType() const override { return ECollectionType::Component; }
    virtual Ptr<EComponentCollection> CloneImp() const override { return new EComponentCollection(*this); }
};
}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentCollection)