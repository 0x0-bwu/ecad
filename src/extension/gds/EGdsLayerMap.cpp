#ifndef ECAD_HEADER_ONLY
#include "EGdsLayerMap.h"
#endif

#include "generic/tools/StringHelper.hpp"
#include "generic/tools/Parser.hpp"
#include <fstream>
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

ECAD_INLINE EGdsLayerMapParser::~EGdsLayerMapParser()
{
}

ECAD_INLINE bool EGdsLayerMapParser::operator() (const std::string & filename)
{
    std::ifstream fp(filename.c_str());
    if(!fp.good()) return false;

    m_layerMap.Clear();
    bool res = (*this)(fp);
    fp.close();
    return res;
}

ECAD_INLINE bool EGdsLayerMapParser::operator() (std::istream & fp)
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

ECAD_INLINE bool EGdsLayerMapParser::ParseOneLine(const std::string & line, EGdsLayer & layer)
{
    using namespace generic::str;
    using namespace generic::parser;

    //name m/v layertype datatype thickness
    auto split = Split(line, std::string(1, char(32)));
    if(split.size() != 5) return false;
    
    auto name = split[0];
    auto type = StartsWith<CaseInsensitive>(split[1], "m") ? ELayerType::ConductingLayer : ELayerType::DielectricLayer;
    layer = EGdsLayer{ name, std::string{}, type, std::stoi(split[2]), std::stoi(split[3]), std::stod(split[4]) };
    return true;
}

}//namespace gds   
}//namespace ext
}//namespace ecad
