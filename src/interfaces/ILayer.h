#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include "Protocol.h"
#include <string>
namespace ecad {

class IViaLayer;
class IStackupLayer;
class ECAD_API ILayer : public Clonable<ILayer>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~ILayer() = default;
    virtual std::string GetName() const = 0;
    virtual void SetLayerId(ELayerId id) = 0;
    virtual ELayerId GetLayerId() const = 0;
    virtual ELayerType GetLayerType() const = 0;
    virtual Ptr<IStackupLayer> GetStackupLayerFromLayer() = 0;
    virtual Ptr<IViaLayer> GetViaLayerFromLayer() = 0;
};

class ECAD_API IStackupLayer
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IStackupLayer() = default;
    virtual std::string GetName() const = 0;
    virtual void SetLayerId(ELayerId id) = 0;
    virtual ELayerId GetLayerId() const = 0;
    virtual ELayerType GetLayerType() const = 0;
    virtual void SetElevation(FCoord elevation) = 0;
    virtual FCoord GetElevation() const = 0;
    virtual void SetThickness(FCoord thickness) = 0;
    virtual FCoord GetThickness() const = 0;

    virtual void SetConductingMaterial(const std::string & material) = 0;
    virtual const std::string & GetConductingMaterial() const = 0;

    virtual void SetDielectricMaterial(const std::string & material) = 0;
    virtual const std::string & GetDielectricMaterial() const = 0;
};

class ECAD_API IViaLayer
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IViaLayer() = default;
};

}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::ILayer)
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IStackupLayer)
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IViaLayer)