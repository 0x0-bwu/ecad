#include "ELayerCutModelBuilder.h"

#include "utils/ELayoutRetriever.h"
#include "Interface.h"

namespace ecad::model::utils {

ECAD_INLINE ELayerCutModelBuilder::ELayerCutModelBuilder(CPtr<ILayoutView> layout, Ptr<ELayerCutModel> model, ELayerCutModelBuildSettings settings)
 : m_layout(layout), m_model(model), m_settings(std::move(settings))
{
    m_model->m_vScale2Int = std::pow(10, m_settings.layerCutPrecision);
    m_retriever = std::make_unique<ecad::utils::ELayoutRetriever>(m_layout);
}

ECAD_INLINE void ELayerCutModelBuilder::AddShape(ENetId netId, EMaterialId solidMat, EMaterialId holeMat, CPtr<EShape> shape, EFloat elevation, EFloat thickness)
{
    if (m_settings.addCircleCenterAsSteinerPoints) {
        if (EShapeType::Circle == shape->GetShapeType()) {
            auto circle = dynamic_cast<CPtr<ECircle>>(shape);
            m_model->m_steinerPoints.emplace_back(circle->o);
        }
    }
    if (shape->hasHole()) {
        auto pwh = shape->GetPolygonWithHoles();
        AddPolygon(netId, solidMat, std::move(pwh.outline), false, elevation, thickness);
        for (auto iter = pwh.ConstBeginHoles(); iter != pwh.ConstEndHoles(); ++iter)
            AddPolygon(netId, holeMat, std::move(*iter), true, elevation, thickness);
    }
    else AddPolygon(netId, solidMat, shape->GetContour(), false, elevation, thickness);
}

ECAD_INLINE size_t ELayerCutModelBuilder::AddPolygon(ENetId netId, EMaterialId matId, EPolygonData polygon, bool isHole, EFloat elevation, EFloat thickness)
{
    auto layerRange = GetLayerRange(elevation, thickness);
    if (not isValid(layerRange)) return invalidIndex;
    if (isHole == polygon.isCCW()) polygon.Reverse();
    m_model->m_ranges.emplace_back(std::move(layerRange));
    m_model->m_polygons.emplace_back(std::move(polygon));
    m_model->m_materials.emplace_back(matId);
    m_model->m_nets.emplace_back(netId);
    return m_model->m_polygons.size() - 1;
};

ECAD_INLINE bool ELayerCutModelBuilder::AddPowerBlock(EMaterialId matId, EPolygonData polygon, EFloat totalP, EFloat elevation, EFloat thickness, EFloat pwrPosition, EFloat pwrThickness)
{
    auto area = polygon.Area();
    auto index = AddPolygon(ENetId::noNet, matId, std::move(polygon), false, elevation, thickness);
    if (invalidIndex == index) return false;
    EFloat pe = elevation - thickness * pwrPosition;
    EFloat pt = std::min(thickness * pwrThickness, thickness - elevation + pe);
    m_model->m_powerBlocks.emplace(index, ELayerCutModel::PowerBlock(index, GetLayerRange(pe, pt), totalP / area));
    return true;
}

ECAD_INLINE void ELayerCutModelBuilder::AddComponent(CPtr<IComponent> component)
{
    EFloat elevation, thickness;
    auto totalP = component->GetLossPower();
    auto boundary = generic::geometry::toPolygon(component->GetBoundingBox());
    auto material = m_layout->GetDatabase()->FindMaterialDefByName(component->GetComponentDef()->GetMaterial()); { ECAD_ASSERT(material) }
    [[maybe_unused]] auto check = m_retriever->GetComponentHeightThickness(component, elevation, thickness); { ECAD_ASSERT(check) }

    if (totalP > 0)
        AddPowerBlock(material->GetMaterialId(), boundary, totalP, elevation, thickness);
    else AddPolygon(ENetId::noNet, material->GetMaterialId(), boundary, false, elevation, thickness);

    check = m_retriever->GetComponentBallBumpThickness(component, elevation, thickness); { ECAD_ASSERT(check) }
    if (thickness > 0) {
        auto solderMat = m_layout->GetDatabase()->FindMaterialDefByName(component->GetComponentDef()->GetSolderFillingMaterial()); { ECAD_ASSERT(solderMat) }
        AddPolygon(ENetId::noNet, solderMat->GetMaterialId(), boundary, false, elevation, thickness);
        //todo solder ball/bump
    }
}

ECAD_INLINE void ELayerCutModelBuilder::AddBondwire(ELayerCutModel::Bondwire bw)
{
    m_model->m_bondwires.emplace_back(std::move(bw));
}

ECAD_INLINE CPtr<ELayerCutModelBuilder::LayoutRetriever> ELayerCutModelBuilder::GetLayoutRetriever() const
{
    return m_retriever.get();
}

ECAD_INLINE ELayerCutModel::Height ELayerCutModelBuilder::GetHeight(EFloat height) const
{
    return m_model->GetHeight(height);
}

ECAD_INLINE ELayerCutModel::LayerRange ELayerCutModelBuilder::GetLayerRange(EFloat elevation, EFloat thickness) const
{
    return m_model->GetLayerRange(elevation, thickness);
}

} // namespace ecad::model::utils
