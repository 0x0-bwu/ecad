#pragma once
#include "interface/IPadstackDefData.h"
#include "basic/ECadCommon.h"
#include "basic/EShape.h"
namespace ecad {

class ECAD_API EVia
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EVia() = default;
    virtual ~EVia() = default;

    ///Copy
    EVia(const EVia & other);
    EVia & operator= (const EVia & other);

    EFloat rotation = 0;
    EPoint2D offset = EPoint2D(0, 0);
    UPtr<EShape> shape = nullptr;
};

class ECAD_API EPad
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPad() = default;
    virtual ~EPad() = default;

    ///Copy
    EPad(const EPad & other);
    EPad & operator= (const EPad & other);

    std::string lyr;
    EFloat rotation = 0;
    EPoint2D offset = EPoint2D(0, 0);
    UPtr<EShape> shape = nullptr;
};

class ECAD_API EBump
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EBump()= default;
    virtual ~EBump() = default;

    ///Copy
    EBump(const EBump & other);
    EBump & operator= (const EBump & other);

    EFloat thickness = 0;
    UPtr<EShape> shape = nullptr;  
    std::string material;
};

using EBall = EBump;
class ECAD_API EPadstackDefData : public IPadstackDefData
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPadstackDefData() = default;
    virtual ~EPadstackDefData() = default;
    
    std::string GetMaterial() const override;
    void SetMaterial(const std::string & material) override;

    void SetLayers(const std::vector<std::string> & layers) override;
    bool SetPadParameters(ELayerId layerId, UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation) override;
    bool GetPadParameters(ELayerId layerId, CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const override;
    bool SetPadParameters(const std::string & layer, UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation) override;
    bool GetPadParameters(const std::string & layer, CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const override;
    void SetViaParameters(UPtr<EShape> shape, const EPoint2D & offset, EFloat rotation) override;
    void GetViaParameters(CPtr<EShape> & shape, EPoint2D & offset, EFloat & rotation) const override;
    
    void SetTopSolderBumpParameters(UPtr<EShape> shape, EFloat thickness) override;
    bool GetTopSolderBumpParameters(CPtr<EShape> & shape, EFloat & thickness) const override;
    void SetBotSolderBallParameters(UPtr<EShape> shape, EFloat thickness) override;
    bool GetBotSolderBallParameters(CPtr<EShape> & shape, EFloat & thickness) const override;

    void SetTopSolderBumpMaterial(const std::string & material) override;
    const std::string & GetTopSolderBumpMaterial() const override;
    void SetBotSolderBallMaterial(const std::string & material) override;
    const std::string & GetBotSolderBallMaterial() const override;

    bool hasTopSolderBump() const override;
    bool hasBotSolderBall() const override;

protected:
    ELayerId GetPadLayerId(const std::string & layer) const;

protected:
    ///Copy
    virtual Ptr<EPadstackDefData> CloneImp() const override { return new EPadstackDefData(*this); }

protected:
    std::string m_material = sDefaultConductingMat;
    std::pair<EBump, EBall> m_solderBumpBall;
    std::vector<EPad> m_pads;
    EVia m_via;
};

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPadstackDefData)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EBump)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPad)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EVia)
