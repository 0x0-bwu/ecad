#pragma once
#include "ECadAlias.h"
#include <iostream>
namespace ecad {

struct Printable
{
public:
    virtual ~Printable() = default;
    virtual void Print(std::ostream & os) const { PrintImp(os); }
protected:
    virtual void PrintImp(std::ostream & os) const = 0;
};

}//namespace ecad

namespace {
ECAD_ALWAYS_INLINE std::ostream & operator<< (std::ostream & os, const ecad::Printable & printable)
{
    printable.Print(os);
    return os;
}

}//namespace