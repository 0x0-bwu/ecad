#pragma once
#include "basic/ECollection.h"
#include <unordered_map>
#include <set>
namespace ecad {

class INetCollection;
class ICellCollection;
class ILayerCollection;
class IModelCollection;
class IConnObjCollection;
class ICellInstCollection;
class ILayerMapCollection;
class IComponentCollection;
class IPrimitiveCollection;
class IDefinitionCollection;
class IMaterialDefCollection;
class IPadstackDefCollection;
class IComponentDefCollection;
class IHierarchyObjCollection;
class IPadstackInstCollection;
class IPadstackInstCollection;
class IComponentDefPinCollection;
using ECollectionTypes = std::set<ECollectionType>;
class ECAD_API ECollectionCollection : public EUnorderedMapCollection<ECollectionType, UPtr<ICollection> >
{
    using BaseCollection = EUnorderedMapCollection<ECollectionType, UPtr<ICollection> >;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ECollectionCollection();
    virtual ~ECollectionCollection();
    
    ///Copy
    ECollectionCollection(const ECollectionCollection & other);
    ECollectionCollection & operator= (const ECollectionCollection & other);

    Ptr<ICollection> AddCollection(ECollectionType type);
    Ptr<ICollection> GetCollection(ECollectionType type) const;

    Ptr<INetCollection> NetCollection() const;
    Ptr<ICellCollection> CellCollection() const;
    Ptr<ILayerCollection> LayerCollection() const;
    Ptr<IModelCollection> ModelCollection() const;
    Ptr<IConnObjCollection> ConnObjCollection() const;
    Ptr<ICellInstCollection> CellInstCollection() const;
    Ptr<ILayerMapCollection> LayerMapCollection() const;
    Ptr<IComponentCollection> ComponentCollection() const;
    Ptr<IPrimitiveCollection> PrimitiveCollection() const;
    Ptr<IDefinitionCollection> DefinitionCollection() const;
    Ptr<IMaterialDefCollection> MaterialDefCollection() const;
    Ptr<IPadstackDefCollection> PadstackDefCollection() const;
    Ptr<IComponentDefCollection> ComponentDefCollection() const;
    Ptr<IHierarchyObjCollection> HierarchyObjCollection() const;
    Ptr<IPadstackInstCollection> PadstackInstCollection() const;
    Ptr<IComponentDefPinCollection> ComponentDefPinCollection() const;

    size_t Size() const override;
    void Clear() override;
protected:
    virtual void Init() { for (auto type : GetCollectionTypes()) AddCollection(type); }
    virtual ECollectionTypes GetCollectionTypes() const { ECAD_ASSERT(false); return {}; }
    virtual ECollectionType GetType() const override { return ECollectionType::Collection; }
    virtual Ptr<ECollectionCollection> CloneImp() const override { return new ECollectionCollection(*this); }
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ECollectionCollection)