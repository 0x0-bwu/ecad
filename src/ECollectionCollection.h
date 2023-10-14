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

    virtual Ptr<ICollection> AddCollection(ECollectionType type) override;
    virtual Ptr<ICollection> GetCollection(ECollectionType type) const override;

    virtual Ptr<INetCollection> GetNetCollection() const override;
    virtual Ptr<ICellCollection> GetCellCollection() const override;
    virtual Ptr<ILayerCollection> GetLayerCollection() const override;
    virtual Ptr<IConnObjCollection> GetConnObjCollection() const override;
    virtual Ptr<ICellInstCollection> GetCellInstCollection() const override;
    virtual Ptr<ILayerMapCollection> GetLayerMapCollection() const override;
    virtual Ptr<IPrimitiveCollection> GetPrimitiveCollection() const override;
    virtual Ptr<IDefinitionCollection> GetDefinitionCollection() const override;
    virtual Ptr<IMaterialDefCollection> GetMaterialDefCollection() const override;
    virtual Ptr<IPadstackDefCollection> GetPadstackDefCollection() const override;
    virtual Ptr<IComponentDefCollection> GetComponentDefCollection() const override;
    virtual Ptr<IHierarchyObjCollection> GetHierarchyObjCollection() const override;
    virtual Ptr<IPadstackInstCollection> GetPadstackInstCollection() const override;
    virtual Ptr<IComponentDefPinCollection> GetComponentDefPinCollection() const override;

    size_t Size() const override;

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