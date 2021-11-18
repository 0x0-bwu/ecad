#ifndef ECAD_EPADSTACKDEFDATA_H
#define ECAD_EPADSTACKDEFDATA_H
#include "interfaces/IPadstackDefData.h"
#include "ECadCommon.h"
#include "ECadDef.h"
#include "EShape.h"
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

    EValue rotation = 0;
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
    EValue rotation = 0;
    EPoint2D offset = EPoint2D(0, 0);
    UPtr<EShape> shape = nullptr;
};

class ECAD_API EPadstackDefData : public IPadstackDefData
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    EPadstackDefData() = default;
    virtual ~EPadstackDefData() = default;
    
    ///Copy
    EPadstackDefData(const EPadstackDefData & other);
    EPadstackDefData & operator= (const EPadstackDefData & other);

    std::string GetMaterial() const;
    void SetMaterial(const std::string & material);

    void SetLayers(const std::vector<std::string> & layers);
    bool SetPadParameters(ELayerId layerId, UPtr<EShape> shape, const EPoint2D & offset, EValue rotation);
    bool GetPadParameters(ELayerId layerId, CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const;
    bool SetPadParameters(const std::string & layer, UPtr<EShape> shape, const EPoint2D & offset, EValue rotation);
    bool GetPadParameters(const std::string & layer, CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const;
    void SetViaParameters(UPtr<EShape> shape, const EPoint2D & offset, EValue rotation);
    void GetViaParameters(CPtr<EShape> & shape, EPoint2D & offset, EValue & rotation) const;
    
protected:
    ELayerId GetPadLayerId(const std::string & layer) const;

protected:
    ///Copy
    virtual Ptr<EPadstackDefData> CloneImp() const override { return new EPadstackDefData(*this); }

protected:
    std::string m_material = sDefaultConductingMat;
    std::vector<EPad> m_pads;
    EVia m_via;
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPadstackDefData)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EPad)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EVia)

#ifdef ECAD_HEADER_ONLY
#include "EPadstackDefData.cpp"
#endif

#endif//ECAD_EPADSTACKDEFDATA_H