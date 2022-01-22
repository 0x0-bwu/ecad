#ifndef ECAD_EPRIMITIVECOLLECTION_H
#define ECAD_EPRIMITIVECOLLECTION_H
#include "interfaces/IPrimitiveCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
#include "EShape.h"
namespace ecad {

class IText;
class ILayerMap;
class IPrimitive;
using EPrimitiveIterator = EVectorCollectionIterator<UPtr<IPrimitive> >;
class ECAD_API EPrimitiveCollection : public EVectorCollection<UPtr<IPrimitive> >, public IPrimitiveCollection
{
    using BaseCollection = EVectorCollection<UPtr<IPrimitive> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPrimitiveCollection();
    virtual ~EPrimitiveCollection();
    
    ///Copy
    EPrimitiveCollection(const EPrimitiveCollection & other);
    EPrimitiveCollection & operator= (const EPrimitiveCollection & other);

    Ptr<IPrimitive> AddPrimitive(UPtr<IPrimitive> primitive);
    
    Ptr<IPrimitive> CreateGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape);
    Ptr<IText> CreateText(ELayerId layer, const ETransform2D & transform, const std::string & text);

    void Map(CPtr<ILayerMap> lyrMap);

    PrimitiveIter GetPrimitiveIter() const;

    size_t Size() const;
    void Clear();
    
protected:
    ///Copy
    virtual Ptr<EPrimitiveCollection> CloneImp() const override { return new EPrimitiveCollection(*this); }
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPrimitiveCollection)

#ifdef ECAD_HEADER_ONLY
#include "EPrimitiveCollection.cpp"
#endif

#endif//ECAD_EPRIMITIVECOLLECTION_H