#ifndef ECAD_IPADSTACKINSTCOLLECTION_H
#define ECAD_IPADSTACKINSTCOLLECTION_H
#include "ECadCommon.h"
#include "IIterator.h"
#include "ECadDef.h"
#include "Protocol.h"
namespace ecad {

class ILayerMap;
class IPadstackDef;
class IPadstackInst;
class ECAD_API IPadstackInstCollection : public Clonable<IPadstackInstCollection>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPadstackInstCollection() = default;

    virtual Ptr<IPadstackInst> AddPadstackInst(UPtr<IPadstackInst> psInst) = 0;

    virtual Ptr<IPadstackInst> CreatePadstackInst(const std::string & name, CPtr<IPadstackDef> def, ENetId net,
                                                    ELayerId topLyr, ELayerId botLyr, CPtr<ILayerMap> layerMap,
                                                    const ETransform2D & transform) = 0;

    virtual void Map(CPtr<ILayerMap> lyrMap) = 0;

    virtual PadstackInstIter GetPadstackInstIter() const = 0;

    virtual size_t Size() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IPadstackInstCollection)
#endif//ECAD_IPADSTACKINSTCOLLECTION_H