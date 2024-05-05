#include "EGeometryModelExtraction.h"

#include "model/geometry/utils/ELayerCutModelBuilder.h"
#include "model/geometry/ELayerCutModel.h"
#include "utility/ELayoutRetriever.h"
#include "interface/Interface.h"

#include "generic/tools/FileSystem.hpp"


namespace ecad::extraction {

using namespace ecad::model;

ECAD_INLINE UPtr<IModel> EGeometryModelExtraction::GenerateLayerCutModel(Ptr<ILayoutView> layout, const ELayerCutModelExtractionSettings & settings)
{
    ECAD_EFFICIENCY_TRACK("generate layer cut model")

    auto model = new ELayerCutModel;
    model::utils::ELayerCutModelBuilder builder(layout, model, settings);
    auto retriever = builder.GetLayoutRetriever();
    
    [[maybe_unused]] bool check;
    EFloat elevation, thickness;
    std::vector<CPtr<IStackupLayer> > stackupLayers;
    layout->GetStackupLayers(stackupLayers);
    for (auto stackupLayer : stackupLayers) {
        auto dielMat = layout->GetDatabase()->FindMaterialDefByName(stackupLayer->GetDielectricMaterial()); { ECAD_ASSERT(dielMat) }
        check = retriever->GetLayerHeightThickness(stackupLayer->GetLayerId(), elevation, thickness); { ECAD_ASSERT(check) } 
        builder.AddShape(ENetId::noNet, dielMat->GetMaterialId(), EMaterialId::noMaterial, layout->GetBoundary(), elevation, thickness);
    }
    
    auto compIter = layout->GetComponentIter();
    while (auto * comp = compIter->Next()) {
        builder.AddComponent(comp);
    }
    
    const auto & coordUnits = layout->GetDatabase()->GetCoordUnits();
    auto scale2Unit = coordUnits.Scale2Unit();
    auto scale2Meter = coordUnits.toUnit(coordUnits.toCoord(1), ECoordUnits::Unit::Meter);
    std::unordered_map<ELayerId, std::pair<EFloat, EFloat> > layerElevationThicknessMap;
    std::unordered_map<ELayerId, std::pair<EMaterialId, EMaterialId> > layerMaterialMap;
    auto primitives = layout->GetPrimitiveCollection();
    for (size_t i = 0; i < primitives->Size(); ++i) {
        auto prim = primitives->GetPrimitive(i);
        if (auto bondwire = prim->GetBondwireFromPrimitive(); bondwire) {
            ELayerCutModel::Bondwire bw;
            check = retriever->GetBondwireSegmentsWithMinSeg(bondwire, bw.pt2ds, bw.heights, 10); { ECAD_ASSERT(check) }
            auto material = layout->GetDatabase()->FindMaterialDefByName(bondwire->GetMaterial());
            ECAD_ASSERT(material)
            bw.matId = material->GetMaterialId();
            bw.radius = bondwire->GetRadius();
            bw.netId = bondwire->GetNet();
            bw.current = bondwire->GetCurrent();
            bw.scenario = bondwire->GetDynamicPowerScenario();
            builder.AddBondwire(std::move(bw));

            if (bondwire->GetSolderJoints() && bondwire->GetSolderJoints()->GetPadstackDefData()->hasTopSolderBump()) {
                std::string sjMatName;
                auto shape = retriever->GetBondwireStartSolderJointParameters(bondwire, elevation, thickness, sjMatName);
                auto sjMat = layout->GetDatabase()->FindMaterialDefByName(sjMatName); { ECAD_ASSERT(sjMat) }
                if (auto current = bondwire->GetCurrent(); current > 0) {
                    EFloat resistivity;
                    check = sjMat->GetProperty(EMaterialPropId::Resistivity)->GetSimpleProperty(25, resistivity); { ECAD_ASSERT(check) }//wbtest, T
                    auto r = resistivity * thickness / (shape->GetContour().Area() * scale2Unit * scale2Unit * scale2Meter);
                    ELookupTable1D table; table.AddSample(ETemperature::Celsius2Kelvins(25), current * current * r);
                    builder.AddPowerBlock(sjMat->GetMaterialId(), shape->GetContour(), bw.scenario, std::make_shared<ELookupTable1D>(table), elevation, thickness, 0.5);
                }
                else builder.AddShape(bw.netId, sjMat->GetMaterialId(), EMaterialId::noMaterial, shape.get(), elevation, thickness);
            }

            if (bondwire->GetSolderJoints() && bondwire->GetSolderJoints()->GetPadstackDefData()->hasBotSolderBall()) {
                std::string sjMatName;
                auto shape = retriever->GetBondwireEndSolderJointParameters(bondwire, elevation, thickness, sjMatName);
                auto sjMat = layout->GetDatabase()->FindMaterialDefByName(sjMatName); { ECAD_ASSERT(sjMat) }
                if (auto current = bondwire->GetCurrent(); current > 0) {
                    EFloat resistivity;
                    check = sjMat->GetProperty(EMaterialPropId::Resistivity)->GetSimpleProperty(25, resistivity); { ECAD_ASSERT(check) }//wbtest, T
                    auto r = resistivity * thickness / (shape->GetContour().Area() * scale2Unit * scale2Unit * scale2Meter);
                    ELookupTable1D table; table.AddSample(ETemperature::Celsius2Kelvins(25), current * current * r);
                    builder.AddPowerBlock(sjMat->GetMaterialId(), shape->GetContour(), bw.scenario, std::make_shared<ELookupTable1D>(table), elevation, thickness, 0.5);
                }
                builder.AddShape(bw.netId, sjMat->GetMaterialId(), EMaterialId::noMaterial, shape.get(), elevation, thickness);
            }
        }
        else if (auto geom = prim->GetGeometry2DFromPrimitive(); geom) {
            auto shape = geom->GetShape();
            auto netId = prim->GetNet();
            auto lyrId = prim->GetLayer();
            auto matIt = layerMaterialMap.find(lyrId);
            if (matIt == layerMaterialMap.cend()) {
                auto layer = layout->GetLayerCollection()->FindLayerByLayerId(lyrId);
                if (nullptr == layer) { ECAD_ASSERT(false); continue; }
                auto stackupLayer = layer->GetStackupLayerFromLayer();
                ECAD_ASSERT(nullptr != stackupLayer);
                auto condMat = layout->GetDatabase()->FindMaterialDefByName(stackupLayer->GetConductingMaterial());
                auto dielMat = layout->GetDatabase()->FindMaterialDefByName(stackupLayer->GetDielectricMaterial());
                ECAD_ASSERT(condMat && dielMat);
                matIt = layerMaterialMap.emplace(lyrId, std::make_pair(condMat->GetMaterialId(), dielMat->GetMaterialId())).first;
            }
            const auto & [condMatId, dielMatId] = matIt->second;
            auto iter = layerElevationThicknessMap.find(lyrId);
            if (iter == layerElevationThicknessMap.cend()) {
                check = retriever->GetLayerHeightThickness(lyrId, elevation, thickness);
                ECAD_ASSERT(check)
                iter = layerElevationThicknessMap.emplace(lyrId, std::make_pair(elevation, thickness)).first;
            }
            std::tie(elevation, thickness) = iter->second;
            builder.AddShape(netId, condMatId, dielMatId, shape, elevation, thickness);
        }        
    }
    auto psInstIter = layout->GetPadstackInstIter();
    while (auto psInst = psInstIter->Next()){
        auto netId = psInst->GetNet();
        auto defData = psInst->GetPadstackDef()->GetPadstackDefData();
        if (nullptr == defData) continue;

        auto material = layout->GetDatabase()->FindMaterialDefByName(defData->GetMaterial());
        ECAD_ASSERT(nullptr != material);

        ELayerId top, bot;
        psInst->GetLayerRange(top, bot);
        for (int i = std::min(top, bot); i <= std::max(top, bot); ++i) {
            auto lyrId = static_cast<ELayerId>(i);
            auto shape = psInst->GetLayerShape(static_cast<ELayerId>(lyrId));
            auto iter = layerElevationThicknessMap.find(lyrId);
            if (iter == layerElevationThicknessMap.cend()) {
                check = retriever->GetLayerHeightThickness(lyrId, elevation, thickness);
                ECAD_ASSERT(check)
                iter = layerElevationThicknessMap.emplace(lyrId, std::make_pair(elevation, thickness)).first;
            }
            std::tie(elevation, thickness) = iter->second;
            builder.AddShape(netId, material->GetMaterialId(), material->GetMaterialId(), shape.get(), elevation, thickness);
        }
    }

    //imprint box
    for (const auto & box : settings.imprintBox)
        builder.AddImprintBox(coordUnits.toCoord(box));

    model->BuildLayerPolygonLUT(settings.layerTransitionRatio);
    return std::unique_ptr<IModel>(model);
}

}//namespace ecad::extraction