#ifndef ECAD_HEADER_ONLY
#include "EGdsLayerMap.h"
#endif

#include "generic/tools/StringHelper.hpp"
#include "generic/tools/Parser.hpp"

namespace ecad {

namespace ext {
namespace gds {

ECAD_INLINE const std::list<EGdsLayer> & EGdsLayerMap::GetAllLayers() const
{
    return m_layers;
}

ECAD_INLINE CPtr<EGdsLayer> EGdsLayerMap::GetLayer(int id) const
{
    auto iter = m_lyrIdMap.find(id);
    if(iter == m_lyrIdMap.end()) return nullptr;
    else return iter->second;
}

ECAD_INLINE void EGdsLayerMap::AddLayer(EGdsLayer layer)
{
    m_layers.emplace_back(std::move(layer));
    m_lyrIdMap.insert(std::make_pair(m_layers.back().layerId, &(m_layers.back())));
}

ECAD_INLINE void EGdsLayerMap::Clear()
{
    m_layers.clear();
    m_lyrIdMap.clear();
}

ECAD_INLINE EGdsLayerMapParser::EGdsLayerMapParser(EGdsLayerMap & layerMap)
 : m_layerMap(layerMap)
{
}

ECAD_INLINE EGdsHelicLayerMapParser::EGdsHelicLayerMapParser(EGdsLayerMap & layerMap)
 : EGdsLayerMapParser(layerMap)
{
}

ECAD_INLINE bool EGdsHelicLayerMapParser::operator() (const std::string & filename)
{
    std::ifstream fp(filename.c_str());
    if(!fp.good()) return false;

    m_layerMap.Clear();
    bool res = (*this)(fp);
    fp.close();
    return res;
}

ECAD_INLINE bool EGdsHelicLayerMapParser::operator() (std::istream & fp)
{
    std::string line;
    while(!fp.eof()){
        line.clear();
        std::getline(fp, line);
        if(line.empty()) continue;
        if(line.front() == '#') continue;

        EGdsLayer layer;
        if(!ParseOneLine(line, layer)) return false;
        m_layerMap.AddLayer(std::move(layer));
    }
    return true;
}

ECAD_INLINE bool EGdsHelicLayerMapParser::ParseOneLine(const std::string & line, EGdsLayer & layer)
{
    using namespace generic::str;
    using namespace generic::parser;
    auto split = Split(line, char(32));
    if(split.size() != 4) return false;
    
    auto name = split[0];
    auto type = StartsWith<CaseInsensitive>(name, "m") ? ELayerType::ConductingLayer : ELayerType::DielectricLayer;
    layer = EGdsLayer{ name, split[1], std::stoi(split[2]), std::stoi(split[3]) };
    return true;
}

}//namespace gds   
}//namespace ext
}//namespace ecad
