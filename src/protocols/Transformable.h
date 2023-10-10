#pragma once
#include "ETransform.h"
namespace ecad {
struct Transformable2D
{
    virtual ~Transformable2D() = default;

    virtual const ETransform2D & GetTransform() const
    {
        return GetTransformImp();
    }

    virtual void SetTransform(const ETransform2D & transform)
    {
        GetTransformImp() = transform;
    }

    virtual void AddTransform(const ETransform2D & transform)
    {
        GetTransformImp().Append(transform);
    }
protected:
    virtual ETransform2D & GetTransformImp() = 0;
    virtual const ETransform2D & GetTransformImp() const = 0;
};

}//namespace ecad
