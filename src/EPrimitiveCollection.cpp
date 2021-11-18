#ifndef ECAD_HEADER_ONLY
#include "EPrimitiveCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EPrimitiveCollection)
#endif

#include "interfaces/ILayerMap.h"
#include "interfaces/INet.h"
#include "EPrimitive.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT

template <typename Archive>
ECAD_INLINE void EPrimitiveCollection::save(Archive & ar, const unsigned int version) const
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPrimitiveCollection, IPrimitiveCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

template <typename Archive>
ECAD_INLINE void EPrimitiveCollection::load(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EPrimitiveCollection, IPrimitiveCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EPrimitiveCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EPrimitiveCollection::EPrimitiveCollection()
{
    m_type = ECollectionType::Primitive;
}

ECAD_INLINE EPrimitiveCollection::~EPrimitiveCollection()
{
}

ECAD_INLINE EPrimitiveCollection::EPrimitiveCollection(const EPrimitiveCollection & other)
{
    *this = other;
}

ECAD_INLINE EPrimitiveCollection & EPrimitiveCollection::operator= (const EPrimitiveCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE Ptr<IPrimitive> EPrimitiveCollection::AddPrimitive(UPtr<IPrimitive> primitive)
{
    Append(std::move(primitive));
    return Back().get();
}


ECAD_INLINE Ptr<IPrimitive> EPrimitiveCollection::CreateGeometry2D(ELayerId layer, ENetId net, UPtr<EShape> shape)
{
    auto primitive = new EGeometry2D(layer, net, std::move(shape));
    return AddPrimitive(UPtr<IPrimitive>(primitive));
}

ECAD_INLINE Ptr<IText> EPrimitiveCollection::CreateText(ELayerId layer, const ETransform2D & transform, const std::string & text)
{
    auto primitive = new EText(text, layer);
    return dynamic_cast<Ptr<IText> >(AddPrimitive(UPtr<IPrimitive>(primitive)));
}

ECAD_INLINE void EPrimitiveCollection::Map(CPtr<ILayerMap> lyrMap)
{
    auto primIter = GetPrimitiveIter();
    while(auto prim = primIter->Next()){
        prim->SetLayer(lyrMap->GetMappingForward(prim->GetLayer()));
    }
}

ECAD_INLINE PrimitiveIter EPrimitiveCollection::GetPrimitiveIter() const
{
    return PrimitiveIter(new EPrimitiveIterator(*this));
}

ECAD_INLINE size_t EPrimitiveCollection::Size() const
{
    return BaseCollection::Size();
}

}//namesapce ecad
