#pragma once
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

    Ptr<IComponentDefPin> AddPin(UPtr<IComponentDefPin> pin);
    Ptr<IComponentDefPin> CreatePin(const std::string & name, EPoint2D loc, EPinIOType type, CPtr<IPadstackDef> psDef = nullptr, ELayerId lyr = noLayer);

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