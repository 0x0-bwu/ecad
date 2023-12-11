#pragma once
#include "ECadCommon.h"
#include "ECadDef.h"
#include <unordered_map>
#include <istream>
#include <list>
namespace ecad {
namespace ext {
namespace gds {

struct ECAD_API EGdsLayer
{
    std::string name;
    std::string purpose;
    ELayerType type = ELayerType::Invalid;
    int layerId = -1;
    int dataType = 0;
    double thickness;
};

class ECAD_API EGdsLayerMap
{
public:
    EGdsLayerMap() = default;
    virtual ~EGdsLayerMap() = default;
    const std::list<EGdsLayer> & GetAllLayers() const;
    CPtr<EGdsLayer> GetLayer(int id) const;
    void AddLayer(EGdsLayer layer);
    void Clear();
private:
    std::list<EGdsLayer> m_layers;
    std::unordered_map<int, CPtr<EGdsLayer> > m_lyrIdMap;
};

class ECAD_API EGdsLayerMapParser
{
public:
    explicit EGdsLayerMapParser(EGdsLayerMap & layerMap);
    virtual ~EGdsLayerMapParser();

    virtual bool operator() (std::string_view filename);
    virtual bool operator() (std::istream & fp);

protected:
    virtual bool ParseOneLine(const std::string & line, EGdsLayer & layer);
    
protected:
    EGdsLayerMap & m_layerMap;
};

}//namespace gds   
}//namespace ext
}//namespace ecad