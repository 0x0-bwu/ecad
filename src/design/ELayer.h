#pragma once
#include "interface/ILayer.h"
#include "EObject.h"
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

    virtual std::string GetName() const;
    virtual void SetLayerId(ELayerId id);
    virtual ELayerId GetLayerId() const;
    virtual ELayerType GetLayerType() const;

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

    std::string GetName() const override;
    void SetLayerId(ELayerId id) override;
    ELayerId GetLayerId() const override;
    ELayerType GetLayerType() const override;

    void SetElevation(EFloat elevation) override;
    EFloat GetElevation() const override;

    void SetThickness(EFloat thickness) override;
    EFloat GetThickness() const override;

    void SetConductingMaterial(const std::string & material) override;
    const std::string & GetConductingMaterial() const override;

    void SetDielectricMaterial(const std::string & material) override;
    const std::string & GetDielectricMaterial() const override;

protected:
    ///Copy
    virtual Ptr<ELayer> CloneImp() const override { return new EStackupLayer(*this);}
protected:
    EFloat m_elevation = 0;
    EFloat m_thickness = 0;
    std::string m_conductingMat;
    std::string m_dielectricMat;
};

class ECAD_API EViaLayer : public ELayer, public IViaLayer
{
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
    EViaLayer();
public:
    explicit EViaLayer(const std::string & name);
    virtual ~EViaLayer();
    
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