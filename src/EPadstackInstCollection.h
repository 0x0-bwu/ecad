#ifndef ECAD_EPADSTACKINSTCOLLECTION_H
#define ECAD_EPADSTACKINSTCOLLECTION_H
#include "interfaces/IPadstackInstCollection.h"
#include "interfaces/IIterator.h"
#include "EBaseCollections.h"
#include <unordered_map>
#include <vector>
namespace ecad {

class IPadstackInst;
using EPadstackInstIterator = EVectorCollectionIterator<UPtr<IPadstackInst> >;
class ECAD_API EPadstackInstCollection : public EVectorCollection<UPtr<IPadstackInst> >, public IPadstackInstCollection
{
    using BaseCollection = EVectorCollection<UPtr<IPadstackInst> >;
    using BaseCollection::CollectionContainer;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPadstackInstCollection();
    virtual ~EPadstackInstCollection();

    ///Copy
    EPadstackInstCollection(const EPadstackInstCollection & other);
    EPadstackInstCollection & operator= (const EPadstackInstCollection & other);

    Ptr<IPadstackInst> AddPadstackInst(UPtr<IPadstackInst> psInst);
    
    Ptr<IPadstackInst> CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                          ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                          const ETransform2D & transform); 
    
    void Map(CPtr<ILayerMap> lyrMap);

    PadstackInstIter GetPadstackInstIter() const;

    size_t Size() const;
protected:
    ///Copy
    virtual Ptr<EPadstackInstCollection> CloneImp() const override { return new EPadstackInstCollection(*this); }
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPadstackInstCollection)

#ifdef ECAD_HEADER_ONLY
#include "EPadstackInstCollection.cpp"
#endif

#endif//ECAD_EPADSTACKINSTCOLLECTION_H