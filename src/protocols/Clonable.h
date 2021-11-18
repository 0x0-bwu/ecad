#ifndef ECAD_PROTOCOL_CLONABLE_H
#define ECAD_PROTOCOL_CLONABLE_H
#include "ECadAlias.h"
#include <vector>
namespace ecad {
template <typename T>
struct Clonable
{
    virtual ~Clonable() = default;

    UPtr<T> Clone() const
    {
        return UPtr<T>(CloneImp());
    }
protected:
    virtual Ptr<T> CloneImp() const = 0;
};

template <typename T>
ECAD_ALWAYS_INLINE UPtr<T> CloneHelper(const UPtr<T> & t)
{
    if(t != nullptr) return t->Clone();
    return nullptr;
}

template <typename T>
ECAD_ALWAYS_INLINE std::vector<UPtr<T> > CloneHelper(const std::vector<UPtr<T> > & vt)
{
    std::vector<UPtr<T> > res;
    for(const auto & t : vt)
        res.push_back(CloneHelper(t));
    return res;
}
}//namespace ecad
#endif//ECAD_PROTOCOL_CLONABLE_H