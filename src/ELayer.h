#ifndef ECAD_ELAYER_H
#define ECAD_ELAYER_H
#include "interfaces/ILayer.h"
#include "EObject.h"
#include "ECadDef.h"
namespace ecad {
class ICell;
class IViaLayer;
class IStackupLayer;
class ECAD_API ELayer : public EObject, public ILayer
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ELayer();
    explicit ELayer(std::string name, ELayerId id = noLayer);
    virtual ~ELayer();

    ///Copy
    ELayer(const ELayer & other);
    ELayer & operator= (const ELayer & other);

    std::string GetName() const;
    void SetLayerId(ELayerId id);
    ELayerId GetLayerId() const;
    ELayerType GetLayerType() const;

    Ptr<IStackupLayer> GetStackupLayerFromLayer();
    Ptr<IViaLayer> GetViaLayerFromLayer();

protected:
    ELayerId m_layerId;
    ELayerType m_layerType;
};

ECAD_ALWAYS_INLINE std::string ELayer::GetName() const
{
    return EObject::GetName();
}

class ECAD_API EStackupLayer : public ELayer, public IStackupLayer
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EStackupLayer();
public:
    EStackupLayer(const std::string & name, ELayerType type);
    virtual ~EStackupLayer();

    ///Copy
    EStackupLayer(const EStackupLayer & other);
    EStackupLayer & operator= (const EStackupLayer & other);

    void SetElevation(FCoord elevation);
    FCoord GetElevation() const;

    void SetThickness(FCoord thickness);
    FCoord GetThickness() const;
protected:
    ///Copy
    virtual Ptr<ELayer> CloneImp() const override { return new EStackupLayer(*this);}
protected:
    FCoord m_elevation = 0;
    FCoord m_thickness = 0;
};

class ECAD_API EViaLayer : public ELayer, public IViaLayer
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EViaLayer();
public:
    explicit EViaLayer(const std::string & name);
    virtual ~EViaLayer();

    ///Copy
    EViaLayer(const EViaLayer & other);
    EViaLayer & operator= (const EViaLayer & other);
protected:
    ///Copy
    virtual Ptr<ELayer> CloneImp() const override { return new EViaLayer(*this); }
protected:
    std::string m_upperRefLyr;
    std::string m_lowerRefLyr;
};

}//namespace ecad

ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ELayer)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EStackupLayer)
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::EViaLayer)

#ifdef ECAD_HEADER_ONLY
#include "ELayer.cpp"
#endif

#endif//ECAD_ELAYER_H