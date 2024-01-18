#pragma once
#define ECAD_EXT_GDS_DEBUG_MODE
#include "extension/gds/EGdsObjects.h"
#include "ECadCommon.h"
#include <map>
namespace ecad {

class ICell;
class IDatabase;
class ILayoutView;

namespace ext {
namespace gds {

class ECAD_API ECadExtGdsHandler
{
public:
    explicit ECadExtGdsHandler(const std::string & gdsFile, const std::string & lyrMapFile = std::string{});
    Ptr<IDatabase> CreateDatabase(const std::string & name, Ptr<std::string> err = nullptr);
private:
    ///cell and cell primitives
    void ImportOneCell(const EGdsCell & cell, Ptr<ICell> iCell);
    void ImportOnePolygon(CPtr<EGdsPolygon> polygon, Ptr<ILayoutView> iLayoutView);
    void ImportOnePath(CPtr<EGdsPath> path, Ptr<ILayoutView> iLayoutView);
    void ImportOneText(CPtr<EGdsText> text, Ptr<ILayoutView> iLayoutView);

    ///cell references
    void ImportCellReferences(const EGdsCell & cell, Ptr<ICell> iCell);
    void ImportOneCellReference(CPtr<EGdsCellReference> ref, Ptr<ILayoutView> iLayoutView);
    void ImportOneCellRefArray(CPtr<EGdsCellRefArray> arr, Ptr<ILayoutView> iLayoutView);

    void Reset();
private:
    std::string m_gdsFile;
    std::string m_lyrMapFile;

private:
    // temporary data
    Ptr<IDatabase> m_database = nullptr;
    std::map<EGdsObject::LayerId, std::set<ELayerId> > m_layerIdMap;
};

}//namespace gds
}//namespace ext
}//namespace ecad