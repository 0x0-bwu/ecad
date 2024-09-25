#pragma once
#include "basic/ECadCommon.h"
#include "basic/EShape.h"
namespace ecad {

class ECAD_API IPadstackDefData : public Clonable<IPadstackDefData>
{
    ECAD_SERIALIZATION_ABSTRACT_CLASS_FUNCTIONS_DECLARATION
public:
    virtual ~IPadstackDefData() = default;

    virtual std::string GetMaterial() const = 0;
    virtual void SetMaterial(const std::string & material) = 0;

    virtual void SetLayers(const std::vector<std::string> & layers) = 0;
    virtual bool SetPadParameters(ELayerId layerId, UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation) = 0;
    virtual bool GetPadParameters(ELayerId layerId, CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const = 0;
    virtual bool SetPadParameters(const std::string & layer, UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation) = 0;
    virtual bool GetPadParameters(const std::string & layer, CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const = 0;
    virtual void SetViaParameters(UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation) = 0;
    virtual void GetViaParameters(CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const = 0;

    virtual void SetTopSolderBumpParameters(UPtr<EShape> shape, EFloat thickness) = 0;
    virtual bool GetTopSolderBumpParameters(CPtr<EShape> & shape, EFloat & thickness) const = 0;
    virtual void SetBotSolderBallParameters(UPtr<EShape> shape, EFloat thickness) = 0;
    virtual bool GetBotSolderBallParameters(CPtr<EShape> & shape, EFloat & thickness) const = 0;
    
    virtual void SetTopSolderBumpMaterial(const std::string & material) = 0;
    virtual const std::string & GetTopSolderBumpMaterial() const = 0;
    virtual void SetBotSolderBallMaterial(const std::string & material) = 0;
    virtual const std::string & GetBotSolderBallMaterial() const = 0;
    
    virtual bool hasTopSolderBump() const = 0;
    virtual bool hasBotSolderBall() const = 0;
};
}//namespace ecad
ECAD_SERIALIZATION_ABSTRACT_CLASS(ecad::IPadstackDefData)
