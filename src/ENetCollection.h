#pragma once
#include "interfaces/INetCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
#include "EUidGenerator.h"
namespace ecad {

class INet;
using ENetIterator = EUnorderedMapCollectionIterator<std::string, UPtr<INet> >;
class ECAD_API ENetCollection : public EUnorderedMapCollection<std::string, UPtr<INet> >, public INetCollection
{
    using NetIdNameMap = std::unordered_map<ENetId, std::string>;
    using BaseCollection = EUnorderedMapCollection<std::string, UPtr<INet> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ENetCollection();
    virtual ~ENetCollection();

    ///Copy
    ENetCollection(const ENetCollection & other);
    ENetCollection & operator= (const ENetCollection & other);

    std::string NextNetName(const std::string & name) const;
    
    Ptr<INet> FindNetByName(const std::string & name) const;
    Ptr<INet> FindNetByNetId(ENetId netId) const;
    Ptr<INet> CreateNet(const std::string & name);
    Ptr<INet> AddNet(UPtr<INet> net);

    NetIter GetNetIter() const;
    size_t Size() const;
    void Clear();

protected:
    ///Copy
    Ptr<ENetCollection> CloneImp() const override;

protected:
    void BuildNetIdLUT() const;
    void ResetNetIdLUT();

protected:
    EUidGenerator m_uidGen;
    mutable UPtr<NetIdNameMap> m_netIdNameMap;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ENetCollection)