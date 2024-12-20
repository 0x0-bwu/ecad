#include "EPadstackInstCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPadstackInstCollection)

#include "design/EPadstackInst.h"
#include "interface/IDatabase.h"
#include "interface/ILayerMap.h"
#include "interface/INet.h"
#include <unordered_set>
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void EPadstackInstCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPadstackInstCollection, IPadstackInstCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPadstackInstCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EPadstackInstCollection::EPadstackInstCollection()
{
}

ECAD_INLINE EPadstackInstCollection::~EPadstackInstCollection()
{
}

ECAD_INLINE EPadstackInstCollection::EPadstackInstCollection(const EPadstackInstCollection & other)
{
    *this = other;
}

ECAD_INLINE EPadstackInstCollection & EPadstackInstCollection::operator= (const EPadstackInstCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IPadstackInst> EPadstackInstCollection::AddPadstackInst(UPtr<IPadstackInst> psInst)
{
    Append(std::move(psInst));
    return Back().get();
}


ECAD_INLINE Ptr<IPadstackInst> EPadstackInstCollection::CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                                                            ELayerId topLyr,ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                                                            const ETransform2D & transform)
{
    auto inst = new EPadstackInst(name, def, net);
    inst->SetLayerRange(topLyr, botLyr);
    inst->SetLayerMap(layerMap);
    inst->SetTransform(transform);

    return AddPadstackInst(UPtr<IPadstackInst>(inst));
}

ECAD_INLINE void EPadstackInstCollection::Map(CPtr<ILayerMap> lyrMap)
{
    ELayerId top, bot;
    std::unordered_map<CPtr<ILayerMap>, CPtr<ILayerMap> > lyrMaps;
    auto psInstIter = GetPadstackInstIter();
    while(auto psInst = psInstIter->Next()){
        psInst->GetLayerRange(top, bot);
        psInst->SetLayerRange(lyrMap->GetMappingForward(top), lyrMap->GetMappingForward(bot));
        
        auto origin = psInst->GetLayerMap();
        if(!lyrMaps.count(origin)){
            auto newMap = origin->Clone();
            auto database = origin->GetDatabase();
            ECAD_ASSERT(database != nullptr)
            auto newName = database->GetNextDefName(newMap->GetName(), EDefinitionType::LayerMap);
            newMap->SetName(newName);
            newMap->MappingLeft(lyrMap);
            lyrMaps.insert(std::make_pair(origin, newMap.get()));
            [[maybe_unused]] auto res = const_cast<Ptr<IDatabase>>(database)->AddLayerMap(std::move(newMap));
            ECAD_ASSERT(res)
        }
        psInst->SetLayerMap(lyrMaps.at(origin));
    }
}

ECAD_INLINE PadstackInstIter EPadstackInstCollection::GetPadstackInstIter() const
{
    return PadstackInstIter(new EPadstackInstIterator(*this));
}

ECAD_INLINE size_t EPadstackInstCollection::Size() const
{
    return BaseCollection::Size();
}


}//namespace ecad
