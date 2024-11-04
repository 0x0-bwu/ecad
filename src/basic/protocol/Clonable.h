#pragma once
#include "basic/ECadAlias.h"
#include <vector>
namespace ecad {
template <typename T, typename Deleter = std::default_delete<T> >
struct Clonable
{
    virtual ~Clonable() = default;

    UPtr<T> Clone() const
    {
        return UPtr<T, Deleter>(CloneImp());
    }
protected:
    virtual Ptr<T> CloneImp() const = 0;
};

template <typename T, typename Deleter = std::default_delete<T> >
ECAD_ALWAYS_INLINE UPtr<T, Deleter> CloneHelper(const UPtr<T, Deleter> & t)
{
    if(t != nullptr) return t->Clone();
    return nullptr;
}

template <typename T, typename Deleter = std::default_delete<T> >
ECAD_ALWAYS_INLINE std::vector<UPtr<T, Deleter> > CloneHelper(const std::vector<UPtr<T, Deleter> > & vt)
{
    std::vector<UPtr<T, Deleter> > res;
    res.reserve(vt.size());
    for(const auto & t : vt)
        res.push_back(CloneHelper<T, Deleter>(t));
    return res;
}
}//namespace ecad