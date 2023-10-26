#include "ECadExtDmcDomHandler.h"

#include "generic/geometry/Utility.hpp"
#include "generic/tools/StringHelper.hpp"
#include "generic/tools/FileSystem.hpp"
#include "generic/tools/Format.hpp"
#include "generic/math/Numbers.hpp"
#include "generic/tools/Tools.hpp"

#include "EDataMgr.h"
#include <tuple>
namespace ecad::ext::dmcdom {

using namespace generic;
namespace fmt = generic::fmt;
ECAD_INLINE bool ParseDomLine(const std::string & line , std::vector<EPoint2D> & points, EValue scale)
{
    double x{0}, y{0};
    std::stringstream(line) >> x >> y;
    points.emplace_back(x * scale, y * scale);
    return true;
}

ECAD_INLINE bool ParseDmcLine(const std::string & line, std::vector<EDmcData> & record, EValue scale)
{
    EDmcData data;

    auto getSiglyrs = [](const std::string & viaLayer, int & lyr1, int & lyr2)
    {
        if(viaLayer.size() < 1) return false;
        if(viaLayer.size() == 1) { lyr1 = lyr2 = -1; return true; }
        auto res = str::Split(viaLayer.substr(1), "-");
        if(res.size() != 2) return false;
        lyr1 = std::stoi(res[0]);
        lyr2 = std::stoi(res[1]);
        return true;
    };
    
    int dark;
    double llx, urx, lly, ury;
    std::string sigLyr;
    std::stringstream ss(line);
    ss >> data.points >> dark >> data.layerId >> llx >> urx >> lly >> ury;
    ss >> data.netId >> data.netName >> sigLyr >> data.lyrName;
    
    if(sigLyr.empty()) return false;
    data.isHole = dark == 0;
    data.isVia = sigLyr.front() == 'V';
    if(!data.isVia) data.sigLyr1 = std::stoi(sigLyr);
    else {
        if(!getSiglyrs(sigLyr, data.sigLyr1, data.sigLyr2))
            return false;
    }

    data.bbox[0][0] = llx * scale;
    data.bbox[1][0] = urx * scale;
    data.bbox[0][1] = lly * scale;
    data.bbox[1][1] = ury * scale;

    record.emplace_back(std::move(data));
    return true;
}

ECAD_INLINE ECadExtDmcDomHandler::ECadExtDmcDomHandler(const std::string & dmc, const std::string & dom, ECoordUnits units)
 : m_dmc(dmc), m_dom(dom), m_units(units) {}

ECAD_INLINE SPtr<IDatabase> ECadExtDmcDomHandler::CreateDatabase(const std::string & name, std::string * err)
{
    auto & mgr = EDataMgr::Instance();
    if(mgr.OpenDatabase(name)){
        if(err) *err = fmt::Fmt2Str("Error: database %1% is already exist.", name);
        return nullptr;
    }
    
    auto points = UPtr<std::vector<EPoint2D> >(new std::vector<EPoint2D>());
    if(!ParseDomFile(m_dom, *points, err)) return nullptr;

    auto record = UPtr<std::vector<EDmcData> >(new std::vector<EDmcData>());
    if(!ParseDmcFile(m_dmc, *record, err)) return nullptr;

    auto database = mgr.CreateDatabase(name);
    if(nullptr == database) return nullptr;

    database->SetCoordUnits(m_units);

    auto cell = mgr.CreateCircuitCell(database, name);
    auto layout = cell->GetLayoutView();

    std::unordered_map<int, ENetId > netMap;
    std::unordered_map<int, ELayerId > lyrMap;
    std::unordered_map<int, int> sigLyrMap;//[sigLyrId, lyrId]
    std::unordered_map<std::string, ELayerId > lyrNameMap;
    std::vector<std::tuple<int, int, int, UPtr<EShape> > > vias;//[net, sigLyr1, sigLyr2, shape]

    EBox2D bbox;
    auto curr = record->begin();
    auto next = curr; next++;
    auto end  = record->end();
    auto ptBegin = points->begin();
    auto ptEnd = points->begin();
    while(curr != end) {
        const EDmcData & data = *curr;
        if(data.isHole) { curr = next; next++; continue; }

        //boundary
        bbox |= data.bbox;

        //geometry
        std::advance(ptEnd, data.points);
        PolygonWithHoles2D<ECoord> pwh;
        pwh.outline.Insert(pwh.outline.End(), std::make_move_iterator(ptBegin), std::make_move_iterator(ptEnd));
        ptBegin = ptEnd;
        while(next != end && next->isHole){
            curr = next; next++;
            const EDmcData & holeData = *curr;

            std::advance(ptEnd, holeData.points);
            Polygon2D<ECoord> hole;
            hole.Insert(hole.End(), std::make_move_iterator(ptBegin), std::make_move_iterator(ptEnd));
            pwh.holes.emplace_back(std::move(hole));
            ptBegin = ptEnd;
        }
        
        UPtr<EShape> shape = nullptr;
        if(pwh.hasHole())
            shape = mgr.CreateShapePolygonWithHoles(std::move(pwh));
        else shape = mgr.CreateShapePolygon(std::move(pwh.outline));

        //net
        if(!netMap.count(data.netId)){ 
            auto net = mgr.CreateNet(layout, data.netName);
            netMap.insert(std::make_pair(data.netId, net->GetNetId()));
        }

        //layer
        if(data.isVia){
            vias.push_back(std::make_tuple(data.netId, data.sigLyr1, data.sigLyr2, std::move(shape)));
        }
        else {
            if(!lyrMap.count(data.layerId)){
                auto lyrType = data.isVia ? ELayerType::DielectricLayer : ELayerType::ConductingLayer;
                auto lyr = mgr.CreateStackupLayer(data.lyrName, lyrType, 0, 0);
                auto lyrId = layout->AppendLayer(std::move(lyr));
                lyrMap.insert(std::make_pair(data.layerId, lyrId));
            }
            if(!sigLyrMap.count(data.sigLyr1)) sigLyrMap.insert(std::make_pair(data.sigLyr1, data.layerId));
            mgr.CreateGeometry2D(layout, lyrMap.at(data.layerId), netMap.at(data.netId), std::move(shape));
        }        
        curr = next; next++;
    }

    //boundary
    auto boundary = new EPolygon;
    boundary->shape = generic::geometry::toPolygon(bbox);
    layout->SetBoundary(UPtr<EPolygon>(boundary));

    std::vector<CPtr<ILayer> > layers;
    layout->GetStackupLayers(layers);

    int toId = 0;
    std::vector<std::string> layerNames;
    auto layerMap = mgr.CreateLayerMap(database, name);
    for(const auto & layer : layers){
        if(layer->GetLayerType() == ELayerType::DielectricLayer) continue;
        layerMap->SetMapping(layer->GetLayerId(), static_cast<ELayerId>(toId));
        layerNames.push_back(layer->GetName());
        toId++;
    }
    
    size_t i = 1;
    for(auto & via : vias){
        auto psDefData = mgr.CreatePadstackDefData();
        psDefData->SetLayers(layerNames);
        psDefData->SetViaParameters(std::move(std::get<3>(via)), EPoint2D(0, 0), 0);
        auto name = database->GetNextDefName(sPadstack, EDefinitionType::PadstackDef);
        auto psDef = mgr.CreatePadstackDef(database, name);
        psDef->SetPadstackDefData(std::move(psDefData));

        std::string viaName = "via_" + std::to_string(i++);
        auto net = netMap.at(std::get<0>(via));
        auto topLyr = lyrMap.at(sigLyrMap.at(std::get<1>(via)));
        auto botLyr = lyrMap.at(sigLyrMap.at(std::get<2>(via)));
        auto transform = makeETransform2D(1, 0, EPoint2D(0, 0));
        mgr.CreatePadstackInst(layout, viaName, psDef, net, topLyr, botLyr, layerMap, transform);
    }
    layout->AddDefaultDielectricLayers();   
    return database;
}

ECAD_INLINE bool ECadExtDmcDomHandler::ParseDomFile(const std::string & filename, std::vector<EPoint2D> & points, std::string * err)
{
    std::ifstream in(filename);
    if(!in.is_open()) return false;
    
    auto scale = m_units.Scale2Coord();
    points.clear();
    size_t count = 0;
    std::string line;
    while(!in.eof()){
        count++;
        line.clear();
        std::getline(in, line);
        if(line.empty()) continue;

        if(!ParseDomLine(line, points, scale)){
            if(err) *err = generic::fmt::Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
            return false;
        }
    }
    in.close();
    return true;
}

ECAD_INLINE bool ECadExtDmcDomHandler::ParseDmcFile(const std::string & filename, std::vector<EDmcData> & record, std::string * err)
{
    std::ifstream in(filename);
    if(!in.is_open()) return false;
    
    auto scale = m_units.Scale2Coord();
    record.clear();
    size_t count = 0;
    std::string line;
    while(!in.eof()){
        count++;
        line.clear();
        std::getline(in, line);
        if(line.empty()) continue;

        if(!ParseDmcLine(line, record, scale)){
            if(err) *err = fmt::Fmt2Str("Error: fail to parse file %1% at line %2%.", filename, count);
            return false;
        }
    }
    in.close();
    return true;
}

}//namespace ecad::ext::dmcdom