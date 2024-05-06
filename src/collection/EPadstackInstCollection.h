#pragma once
#include "interface/IPadstackInstCollection.h"
#include "interface/IIterator.h"
#include "basic/ECollection.h"
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

    Ptr<IPadstackInst> AddPadstackInst(UPtr<IPadstackInst> psInst) override;
    
    Ptr<IPadstackInst> CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                          ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                          const ETransform2D & transform) override; 
    
    void Map(CPtr<ILayerMap> lyrMap) override;

    PadstackInstIter GetPadstackInstIter() const override;

    size_t Size() const override;
protected:
    ///Copy
    virtual Ptr<EPadstackInstCollection> CloneImp() const override { return new EPadstackInstCollection(*this); }
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPadstackInstCollection)
