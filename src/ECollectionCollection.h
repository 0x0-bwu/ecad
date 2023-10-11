#pragma once
#include "interfaces/ICollectionCollection.h"
#include "EBaseCollections.h"
#include <unordered_map>
#include <string>
namespace ecad {

class ECAD_API ECollectionCollection : public EUnorderedMapCollection<ECollectionType, UPtr<ICollection> >, public ICollectionCollection
{
    using BaseCollection = EUnorderedMapCollection<ECollectionType, UPtr<ICollection> >;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECollectionCollection();
    explicit ECollectionCollection(std::string name);
    virtual ~ECollectionCollection();
    
    ///Copy
    ECollectionCollection(const ECollectionCollection & other);
    ECollectionCollection & operator= (const ECollectionCollection & other);

    virtual Ptr<ICollection> AddCollection(ECollectionType type);
    virtual Ptr<ICollection> GetCollection(ECollectionType type) const;

    virtual Ptr<INetCollection> GetNetCollection() const;
    virtual Ptr<ICellCollection> GetCellCollection() const;
    virtual Ptr<ILayerCollection> GetLayerCollection() const;
    virtual Ptr<IConnObjCollection> GetConnObjCollection() const;
    virtual Ptr<ICellInstCollection> GetCellInstCollection() const;
    virtual Ptr<ILayerMapCollection> GetLayerMapCollection() const;
    virtual Ptr<IPrimitiveCollection> GetPrimitiveCollection() const;
    virtual Ptr<IDefinitionCollection> GetDefinitionCollection() const;
    virtual Ptr<IMaterialDefCollection> GetMaterialDefCollection() const;
    virtual Ptr<IPadstackDefCollection> GetPadstackDefCollection() const;
    virtual Ptr<IComponentDefCollection> GetComponentDefCollection() const;
    virtual Ptr<IHierarchyObjCollection> GetHierarchyObjCollection() const;
    virtual Ptr<IPadstackInstCollection> GetPadstackInstCollection() const;
    virtual Ptr<IComponentDefPinCollection> GetComponentDefPinCollection() const;

    size_t Size() const;

    const std::string & GetName() const;
    std::string sUuid() const;

protected:
    ///Copy
    virtual Ptr<ECollectionCollection> CloneImp() const override { return new ECollectionCollection(*this); }
};

ECAD_ALWAYS_INLINE const std::string & ECollectionCollection::GetName() const
{
    return ECollection::GetName();
}

ECAD_ALWAYS_INLINE std::string ECollectionCollection::sUuid() const
{
    return ECollection::sUuid();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECollectionCollection)