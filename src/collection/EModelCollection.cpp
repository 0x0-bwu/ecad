#include "EModelCollection.h"
ECAD_SERIALIZATION_CLASS_EXPORT_IMP(ecad::EModelCollection)

#include "interface/IModel.h"
namespace ecad {

#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    
template <typename Archive>
ECAD_INLINE void EModelCollection::serialize(Archive & ar, const unsigned int version)
{
    ECAD_UNUSED(version)
    boost::serialization::void_cast_register<EModelCollection, IModelCollection>();
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BaseCollection);
}

ECAD_SERIALIZATION_FUNCTIONS_IMP(EModelCollection)
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT

ECAD_INLINE EModelCollection::EModelCollection()
{
    m_type = ECollectionType::Model;
}

ECAD_INLINE EModelCollection::~EModelCollection()
{
}

ECAD_INLINE EModelCollection::EModelCollection(const EModelCollection & other)
{
    *this = other;
}

ECAD_INLINE EModelCollection & EModelCollection::operator= (const EModelCollection & other)
{
    BaseCollection::operator=(other);
    return *this;
}

ECAD_INLINE CPtr<IModel> EModelCollection::FindModel(EModelType type) const
{
    if (not BaseCollection::Count(type)) return nullptr;
    return BaseCollection::At(type).get();
}

ECAD_INLINE bool EModelCollection::AddModel(UPtr<IModel> model)
{
    if (nullptr == model) return false;
    return BaseCollection::Insert(model->GetModelType(), std::move(model), true);
}

ECAD_INLINE size_t EModelCollection::Size() const
{
    return BaseCollection::Size();
}

ECAD_INLINE void EModelCollection::Clear()
{
    BaseCollection::Clear();
}

}//namesapce ecad