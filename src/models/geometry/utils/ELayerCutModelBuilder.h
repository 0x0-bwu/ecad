#pragma once
#include "ECadCommon.h"
#include "ECadSettings.h"
#include "models/geometry/ELayerCutModel.h"
#include "interfaces/IModel.h"

namespace ecad {

class IComponent;
class ILayoutView;
namespace utils {
class ELayoutRetriever;
}

namespace model::utils {

class ECAD_API ELayerCutModelBuilder
{
public:
    using LayoutRetriever = ecad::utils::ELayoutRetriever;
    ELayerCutModelBuilder(CPtr<ILayoutView> layout, Ptr<ELayerCutModel> model, ELayerCutModelBuildSettings settings);
    virtual ~ELayerCutModelBuilder() = default;

    void AddShape(ENetId netId, EMaterialId solidMat, EMaterialId holeMat, CPtr<EShape> shape, EFloat elevation, EFloat thickness);
    size_t AddPolygon(ENetId netId, EMaterialId matId, EPolygonData polygon, bool isHole, EFloat elevation, EFloat thickness);
    bool AddPowerBlock(EMaterialId matId, EPolygonData polygon, SPtr<ELookupTable1D> power, EFloat elevation, EFloat thickness, EFloat pwrPosition = 0.1, EFloat pwrThickness = 0.1);
    void AddComponent(CPtr<IComponent> component);
    void AddBondwire(ELayerCutModel::Bondwire bw);
    void AddImprintBox(const EBox2D & box);
    CPtr<LayoutRetriever> GetLayoutRetriever() const; 

protected:
    ELayerCutModel::Height GetHeight(EFloat height) const;
    ELayerCutModel::LayerRange GetLayerRange(EFloat elevation, EFloat thickness) const;

protected:
    CPtr<ILayoutView> m_layout{nullptr};
    Ptr<ELayerCutModel> m_model{nullptr};
    ELayerCutModelBuildSettings m_settings;
    UPtr<LayoutRetriever> m_retriever;
};
} // namespace model::utils
} // namespace ecad;
