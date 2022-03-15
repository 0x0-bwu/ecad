#ifndef ECAD_ECOMPONENTDEFPINCOLLECTION_H
#define ECAD_ECOMPONENTDEFPINCOLLECTION_H
#include "interfaces/IComponentDefPinCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
namespace ecad {

class IComponentDefPin;
using EComponentDefPinIterator = EVectorCollectionIterator<UPtr<IComponentDefPin> >;
class ECAD_API EComponentDefPinCollection : public EVectorCollection<UPtr<IComponentDefPin> >, public IComponentDefPinCollection
{
    using BaseCollection = EVectorCollection<UPtr<IComponentDefPin> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EComponentDefPinCollection();
    virtual ~EComponentDefPinCollection();

    ///Copy
    EComponentDefPinCollection(const EComponentDefPinCollection & other);
    EComponentDefPinCollection & operator= (const EComponentDefPinCollection & other);

    ComponentDefPinIter GetComponentDefPinIter() const;
    size_t Size() const;
    void Clear();

protected:
    ///Copy
    Ptr<EComponentDefPinCollection> CloneImp() const override;
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EComponentDefPinCollection)

#ifdef ECAD_HEADER_ONLY
#include "EComponentDefPinCollection.cpp"
#endif

#endif//ECAD_ECOMPONENTDEFPINCOLLECTION_H