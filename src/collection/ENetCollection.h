#pragma once
#include "interface/INetCollection.h"
#include "interface/IIterator.h"
#include "basic/EUidGenerator.h"
#include "basic/ECollection.h"
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

    std::string NextNetName(const std::string & name) const override;
    
    Ptr<INet> FindNetByName(const std::string & name) const override;
    Ptr<INet> FindNetByNetId(ENetId netId) const override;
    Ptr<INet> CreateNet(const std::string & name) override;
    Ptr<INet> AddNet(UPtr<INet> net) override;

    NetIter GetNetIter() const override;
    size_t Size() const override;
    void Clear() override;

protected:
    void BuildNetIdLUT() const;
    void ResetNetIdLUT();

protected:
    virtual ECollectionType GetType() const override { return ECollectionType::Net; }
    virtual Ptr<ENetCollection> CloneImp() const override;

protected:
    EUidGenerator m_uidGen;
    mutable UPtr<NetIdNameMap> m_netIdNameMap;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ENetCollection)