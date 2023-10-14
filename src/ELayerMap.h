#pragma once
#include "interfaces/ILayerMap.h"
#include "EDefinition.h"
#include <boost/bimap.hpp>
namespace ecad {

class ECAD_API ELayerMap : public EDefinition, public ILayerMap
{
    using LayerIdMap = boost::bimap<ELayerId, ELayerId>;
    ECAD_SERIALIZATION_FUNCTIONS_DECLARATION
public:
    ELayerMap();
    ELayerMap(const std::string & name, Ptr<IDatabase> database);
    virtual ~ELayerMap();

    ///Copy
    ELayerMap(const ELayerMap & other);
    ELayerMap & operator= (const ELayerMap & other);

    Ptr<IDatabase> GetDatabase() const override;

    EDefinitionType GetDefinitionType() const override;

    void SetName(std::string name) override;
    const std::string & GetName() const override;

    void SetMapping(ELayerId from, ELayerId to) override;
    void MappingLeft(CPtr<ILayerMap> layerMap) override;
    void MappingRight(CPtr<ILayerMap> layerMap) override;

    ELayerId GetMappingForward(ELayerId from) const override;
    ELayerId GetMappingBackward(ELayerId to) const override;

protected:
    ///Copy
    virtual Ptr<ELayerMap> CloneImp() const override { return new ELayerMap(*this); }

protected:
    LayerIdMap m_layerIdMap;
    Ptr<IDatabase> m_database;
};

ECAD_ALWAYS_INLINE void ELayerMap::SetName(std::string name)
{
    EObject::SetName(std::move(name));
}

ECAD_ALWAYS_INLINE const std::string & ELayerMap::GetName() const
{
    return EObject::GetName();
}

}//namespace ecad
ECAD_SERIALIZATION_CLASS_EXPORT_KEY(ecad::ELayerMap)