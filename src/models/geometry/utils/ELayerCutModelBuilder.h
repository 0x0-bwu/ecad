#pragma once
#include "ECadCommon.h"
#include "interfaces/IModel.h"

namespace ecad {

class EShape;
class IComponent;
class ILayoutView;
namespace utils {
class ELayoutRetriever;
}

namespace model {

class ELayerCutModel;
namespace utils {

class ECAD_API ELayerCutModelBuilder
{
public:
    using LayoutRetriever = ecad::utils::ELayoutRetriever;
    ELayerCutModelBuilder(CPtr<ILayoutView> layout, Ptr<ELayerCutModel> model, ELayerCutModelBuildSettings settings);
    virtual ~ELayerCutModelBuilder() = default;

    void AddShape(ENetId netId, EMaterialId solidMat, EMaterialId holeMat, CPtr<EShape> shape, EFloat elevation, EFloat thickness);
    size_t AddPolygon(ENetId netId, EMaterialId matId, EPolygonData polygon, bool isHole, EFloat elevation, EFloat thickness);
    bool AddPowerBlock(EMaterialId matId, EPolygonData polygon, EFloat totalP, EFloat elevation, EFloat thickness, EFloat pwrPosition = 0.1, EFloat pwrThickness = 0.1);
    void AddComponent(CPtr<IComponent> component);
    void AddBondwire(ELayerCutModel::Bondwire bw);
    CPtr<LayoutRetriever> GetLayoutRetriever() const; 

protected:
    Height GetHeight(EFloat height) const;
    LayerRange GetLayerRange(EFloat elevation, EFloat thickness) const;

protected:
    CPtr<ILayoutView> m_layout{nullptr};
    Ptr<ELayerCutModel> m_model{nullptr};
    ELayerCutModelBuildSettings m_settings;

    EFloat m_vScale2Int;
    std::unique_ptr<LayoutRetriever> m_retriever;
};
} // namespace utils
} // namespace model
} // namespace ecad
