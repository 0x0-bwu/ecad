#ifndef ECAD_ICOLLECTION_ICOLLECTION_H
#define ECAD_ICOLLECTION_ICOLLECTION_H
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
namespace ecad {

class ICollection;
class INetCollection;
class ILayerCollection;
class IConnObjCollection;
class ICellInstCollection;
class IPrimitiveCollection;
class IHierarchyObjCollection;
class IPadstackInstCollection;
class ECAD_API ICollectionCollection : public Clonable<ICollectionCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ICollectionCollection() = default;

    virtual Ptr<ICollection> AddCollection(ECollectionType type) = 0;
    virtual Ptr<ICollection> GetCollection(ECollectionType type) const = 0;

    virtual Ptr<INetCollection> GetNetCollection() const = 0;
    virtual Ptr<ILayerCollection> GetLayerCollection() const = 0;
    virtual Ptr<IConnObjCollection> GetConnObjCollection() const = 0;
    virtual Ptr<ICellInstCollection> GetCellInstCollection() const = 0;
    virtual Ptr<IPrimitiveCollection> GetPrimitiveCollection() const = 0;
    virtual Ptr<IHierarchyObjCollection> GetHierarchyObjCollection() const = 0;
    virtual Ptr<IPadstackInstCollection> GetPadstackInstCollection() const = 0;
    
    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ICollectionCollection)
#endif//ECAD_ICOLLECTION_ICOLLECTION_H