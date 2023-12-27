#pragma once
#include "generic/geometry/Geometries.hpp"
#include "extension/gds/EGdsRecords.h"
#include "ECadCommon.h"
#include "EShape.h"
#include <unordered_map>
#include <unordered_set>
namespace ecad {

namespace ext {
namespace gds {

using namespace generic::geometry;

struct EGdsObject
{
    using LayerId = int16_t;
    virtual ~EGdsObject() = default;
};

struct EGdsShape : public EGdsObject
{
    using BaseType = EGdsObject;
    using BaseType::LayerId;
    using DataType = int16_t;

    LayerId layer;
    DataType dataType;
    virtual ~EGdsShape() = default;
};

struct EGdsRectangle : public EGdsShape
{
    using Shape = ERectangle;
    Shape shape;
    ~EGdsRectangle() = default;
};

struct EGdsPolygon : public EGdsShape
{
    using Shape = EPolygon;
    Shape shape;
    ~EGdsPolygon() = default;

    void SetPoints(const std::vector<EPoint2D> & points)
    {
        shape.SetPoints(points);
    }
};

struct EGdsPath : public EGdsShape
{
    using Shape = EPath;
    Shape shape;
    ~EGdsPath() = default;
    
    void SetPoints(const std::vector<EPoint2D> & points)
    {
        shape.SetPoints(points);
    }

    void SetPathType(int type)
    {
        shape.SetType(type);
    }

    void SetWidth(ECoord width)
    {
        shape.SetWidth(width);
    }
};

struct EGdsText : public EGdsShape
{
    int width;
    int strans;
    int textType;
    int presentation;
    std::string text;
    EPoint2D position;
    EFloat rotation = 0;
    EFloat scale = 1;
    ~EGdsText() = default;
};

struct EGdsCellReference : public EGdsObject
{
    std::string refCell;
    EPoint2D position;
    EFloat rotation = 0;
    EFloat scale = 1;
    int strans;
};

struct EGdsCellRefArray : public EGdsObject
{
    std::string refCell;
    std::vector<EPoint2D> positions;
    EFloat rotation = 0;
    EFloat scale = 1;
    size_t cols, rows;
    int spacing[2];
    int strans;
};

struct EGdsCell : public EGdsObject
{
    using Object = std::pair<EGdsRecords::EnumType, UPtr<EGdsObject> >;
    std::string name;
    std::vector<Object> objects;

    void AddPolygon(int layer, int dataType, const std::vector<EPoint2D> & points)
    {
        auto polygon = new EGdsPolygon;
        polygon->layer = layer;
        polygon->dataType = dataType;
        polygon->SetPoints(points);
        objects.push_back(std::make_pair(EGdsRecords::BOUNDARY, UPtr<EGdsObject>(polygon)));
    }
    
    void AddPath(int layer, int dataType, int pathType, int width, std::vector<EPoint2D> & points)
    {
        auto path = new EGdsPath;
        path->layer = layer;
        path->dataType = dataType;
        path->SetPoints(points);
        path->SetPathType(pathType);
        path->SetWidth(width);
        objects.push_back(std::make_pair(EGdsRecords::PATH, UPtr<EGdsObject>(path)));
    }

    void AddText(int layer, int dataType, int textType, const std::string & str,
                const EPoint2D & position, int width, int presentation,
                double rotation, double scale, int strans)
    {
        auto text = new EGdsText;
        text->layer = layer;
        text->dataType = dataType;
        text->textType = textType;
        text->text = str;
        text->position = position;
        text->width = width;
        text->presentation = presentation;
        text->rotation = rotation;
        text->scale = scale;
        text->strans = strans;
        objects.push_back(std::make_pair(EGdsRecords::TEXT, UPtr<EGdsObject>(text))); 
    }

    void AddCellReference(const std::string & refCell, const EPoint2D & position, double rotation, double scale, int strans)
    {
        auto cellRef = new EGdsCellReference;
        cellRef->refCell = refCell;
        cellRef->position = position;
        cellRef->rotation = rotation;
        cellRef->scale = scale;
        cellRef->strans = strans;
        objects.push_back(std::make_pair(EGdsRecords::SREF, UPtr<EGdsObject>(cellRef)));
    }

    void AddCellRefArray(const std::string & refCell, size_t cols, size_t rows, int spacing[2], const std::vector<EPoint2D> & positions, double rotation, double scale, int strans)
    {
        auto cellRefArray = new EGdsCellRefArray;
        cellRefArray->refCell = refCell;
        cellRefArray->cols = cols;
        cellRefArray->rows = rows;
        cellRefArray->spacing[0] = spacing[0];
        cellRefArray->spacing[1] = spacing[1];
        cellRefArray->positions = positions;
        cellRefArray->rotation = rotation;
        cellRefArray->scale = scale;
        cellRefArray->strans = strans;
        objects.push_back(std::make_pair(EGdsRecords::AREF, UPtr<EGdsObject>(cellRefArray)));
    }
};

struct EGdsDB : public EGdsObject
{
    std::string header;
    std::string libName;
    ECoordUnits coordUnits;
    std::vector<EGdsCell> cells;
    std::unordered_set<EGdsObject::LayerId> layers;
    std::unordered_map<std::string, size_t> cellIdxMap;

    void SetHeader(int h)
    {
        header = std::to_string(h);
    }

    void SetHeader(const std::string & _header)
    {
        header = _header;
    }

    void SetLibName(const std::string & lib)
    {
        libName = lib;
    }

    void SetUnit(double unit)
    {
        coordUnits.SetUnit(unit);
    }

    void SetPrecision(double precision)
    {
        coordUnits.SetPrecision(precision);
    }

    bool AddCell(const std::string & name)
    {
        if(cellIdxMap.count(name)) return false;

        cellIdxMap.insert(std::make_pair(name, cells.size()));
        cells.push_back(EGdsCell{}); 
        cells.back().name = name;
        return true;
    }

    const std::unordered_set<EGdsObject::LayerId> & Layers() const
    {
        return layers;
    }

    std::unordered_set<EGdsObject::LayerId> & Layers()
    {
        return layers;
    }

    const std::vector<EGdsCell> & Cells() const
    {
        return cells;
    }

    std::vector<EGdsCell> & Cells()
    {
        return cells;
    }
};

}//namespace gds   
}//namespace ext
}//namespace ecad