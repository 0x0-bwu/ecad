#ifndef ECAD_EXT_GDS_EGDSLAYMAP_H
#define ECAD_EXT_GDS_EGDSLAYMAP_H
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
    int         layerId  = -1;
    int         dataType = -1;
    ELayerType type = ELayerType::Invalid;
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
    EGdsLayerMapParser(EGdsLayerMap & layerMap);
    virtual ~EGdsLayerMapParser() = default;

    virtual bool operator() (const std::string & filename) = 0;
    virtual bool operator() (std::istream & fp) = 0;

protected:
    EGdsLayerMap & m_layerMap;
};

class ECAD_API EGdsHelicLayerMapParser : public EGdsLayerMapParser
{
public:
    EGdsHelicLayerMapParser(EGdsLayerMap & layerMap);
    bool operator() (const std::string & filename);
    bool operator() (std::istream & fp);

private:
    bool ParseOneLine(const std::string & line, EGdsLayer & layer);
};

}//namespace gds   
}//namespace ext
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EGdsLayerMap.cpp"
#endif

#endif//ECAD_EXT_GDS_EGDSLAYMAP_H