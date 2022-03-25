#ifndef ECAD_ESIM_ETHERMALNETWOKREXTRACTION_H
#define ECAD_ESIM_ETHERMALNETWOKREXTRACTION_H
#include "thermal/model/ThermalNetwork.hpp"
#include "models/EThermalModel.h"
#include "ECadSettings.h"
#include "ECadCommon.h"
namespace ecad {
class ILayoutView;
namespace esim {

using namespace emodel;
using namespace thermal::model;
using EThermalNetworkNumType = double;
class ECAD_API EGridThermalNetworkBuilder
{
    using float_t = EThermalNetworkNumType;
public:
    enum class Orientation { Top, Bot, Left, Right, Front, Back };
    explicit EGridThermalNetworkBuilder(const EGridThermalModel & model);
    virtual ~EGridThermalNetworkBuilder();

    UPtr<ThermalNetwork<EThermalNetworkNumType> > Build(const std::vector<float_t> & iniT);

private:
    std::array<float_t, 3> GetConductingMatK(size_t layer, float_t refT) const;
    std::array<float_t, 3> GetDielectircMatK(size_t layer, float_t refT) const;
private:
    size_t GetFlattenIndex(const ESize3D & index) const;
    ESize3D GetGridIndex(size_t index) const;
    ESize3D GetNeighbor(size_t index, Orientation o) const;
    ESize3D GetNeighbor(ESize3D index, Orientation o) const;
    size_t GetFlattenNeighbor(size_t index, Orientation o) const;
    size_t GetFlattenNeighbor(ESize3D index, Orientation o) const;
    static bool isValid(const ESize3D & index);

private:
    const EGridThermalModel & m_model;
    const ESize3D m_size;
};

ECAD_ALWAYS_INLINE size_t EGridThermalNetworkBuilder::GetFlattenIndex(const ESize3D & index) const
{
    return m_size.x * m_size.y * index.z + m_size.y * index.x + index.y;
}

ECAD_ALWAYS_INLINE ESize3D EGridThermalNetworkBuilder::GetGridIndex(size_t index) const
{
    ESize3D gridIndex;
    size_t tmp = m_size.x * m_size.y;
    gridIndex.z = index / tmp;
    tmp = index % tmp;
    gridIndex.y = tmp % m_size.y;
    gridIndex.x = tmp / m_size.y;
    return gridIndex;
}

ECAD_ALWAYS_INLINE bool EGridThermalNetworkBuilder::isValid(const ESize3D & index)
{
    return index.x != invalidIndex && index.y != invalidIndex && index.z != invalidIndex;
}

class ECAD_API EThermalNetworkExtraction
{
    using float_t = double;

    struct ModelIndex
    {
        size_t x = invalidIndex;
        size_t y = invalidIndex;
        size_t z = invalidIndex;
        bool isValid() const
        {
            return (x != invalidIndex) && (y != invalidIndex) && (z != invalidIndex);
        }
    };
    enum class Axis { PX, NX, PY, NY, PZ, NZ };
    enum class Quadrant { I = 1, II = 2, III = 3, IV = 4, V = 5, VI = 6, VII = 7, VIII = 8 };
public:
    void SetExtractionSettings(EThermalNetworkExtractionSettings settings);
    bool GenerateThermalNetwork(Ptr<ILayoutView> layout);
private:
    size_t GetFlattenIndex(const ModelIndex & index) const;
    ModelIndex GetModelIndex(size_t index) const;
    size_t GetNeighbor(size_t index, Axis axis) const;
    size_t GetNeighbor(ModelIndex modelIndex, Axis axis) const;
    ModelIndex GetMetalFractionBlockIndex(size_t index, Quadrant quadrant) const;//bot->up
    bool isBoundaryNode(size_t index) const;
    
private:
    ModelIndex m_modelSize;
    UPtr<ThermalNetwork<float_t> > m_network;
    EThermalNetworkExtractionSettings m_settings;
};

ECAD_ALWAYS_INLINE size_t EThermalNetworkExtraction::GetFlattenIndex(const ModelIndex & index) const
{
    return m_modelSize.x * m_modelSize.y * index.z + m_modelSize.y * index.x + index.y;
}

ECAD_ALWAYS_INLINE EThermalNetworkExtraction::ModelIndex EThermalNetworkExtraction::GetModelIndex(size_t index) const
{
    ModelIndex modelIndex;
    auto size1 = m_modelSize.x * m_modelSize.y;
    modelIndex.z = index / size1;
    auto size2 = index % size1;
    modelIndex.y = size2 % m_modelSize.y;
    modelIndex.x = size2 / m_modelSize.y;
    return modelIndex;
}

ECAD_ALWAYS_INLINE size_t EThermalNetworkExtraction::GetNeighbor(size_t index, Axis axis) const
{
    return GetNeighbor(GetModelIndex(index), axis);
}

ECAD_ALWAYS_INLINE size_t EThermalNetworkExtraction::GetNeighbor(ModelIndex modelIndex, Axis axis) const
{
    switch(axis) {
        case Axis::PX : {
            if(modelIndex.x == (m_modelSize.x - 1)) return invalidIndex;
            modelIndex.x += 1; break;
        }
        case Axis::NX : {
            if(modelIndex.x == 0) return invalidIndex;
            modelIndex.x -= 1; break;
        }
        case Axis::PY : {
            if(modelIndex.y == (m_modelSize.y - 1)) return invalidIndex;
            modelIndex.y += 1; break;
        }
        case Axis::NY : {
            if(modelIndex.y == 0) return invalidIndex;
            modelIndex.y -= 1; break;
        }
        case Axis::PZ : {
            if(modelIndex.z == (m_modelSize.z - 1)) return invalidIndex;
            modelIndex.z += 1; break;
        }
        case Axis::NZ : {
            if(modelIndex.z == 0) return invalidIndex;
            modelIndex.z -= 1; break;
        }
        default : return invalidIndex;
    }
    return GetFlattenIndex(modelIndex);
}

ECAD_ALWAYS_INLINE EThermalNetworkExtraction::ModelIndex EThermalNetworkExtraction::GetMetalFractionBlockIndex(size_t index, Quadrant quadrant) const//bot->up
{
    auto modelIndex = GetModelIndex(index);
    switch(quadrant){
        case Quadrant::I : {
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PX)){
                modelIndex.x = invalidIndex;
            }
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PY)){
                modelIndex.y = invalidIndex;
            }
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PZ)){
                modelIndex.z = invalidIndex;
            }
            break;
        }
        case Quadrant::II : {
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NX)){
                modelIndex.x = invalidIndex;
            }
            else modelIndex.x -= 1;
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PY)){
                modelIndex.y = invalidIndex;
            }
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PZ)){
                modelIndex.z = invalidIndex;
            }
            break;
        }
        case Quadrant::III : {
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NX)){
                modelIndex.x = invalidIndex;
            }
            else modelIndex.x -= 1;
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PY)){
                modelIndex.y = invalidIndex;
            }
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NZ)){
                modelIndex.z = invalidIndex;
            }
            else modelIndex.z -= 1;
            break;
        }
        case Quadrant::IV : {
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PX)){
                modelIndex.x = invalidIndex;
            }
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PY)){
                modelIndex.y = invalidIndex;
            }
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NZ)){
                modelIndex.z = invalidIndex;
            }
            else modelIndex.z -= 1;
            break;
        }
        case Quadrant::V : {
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PX)){
                modelIndex.x = invalidIndex;
            }
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NY)){
                modelIndex.y = invalidIndex;
            }
            else modelIndex.y -= 1;
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PZ)){
                modelIndex.z = invalidIndex;
            }
            break;
        }
        case Quadrant::VI : {
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NX)){
                modelIndex.x = invalidIndex;
            }
            else modelIndex.x -= 1;
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NY)){
                modelIndex.y = invalidIndex;
            }
            else modelIndex.y -= 1;
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PZ)){
                modelIndex.z = invalidIndex;
            }
            break;
        }
        case Quadrant::VII : {
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NX)){
                modelIndex.x = invalidIndex;
            }
            else modelIndex.x -= 1;
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NY)){
                modelIndex.y = invalidIndex;
            }
            else modelIndex.y -= 1;
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NZ)){
                modelIndex.z = invalidIndex;
            }
            else modelIndex.z -= 1;
            break;
        }
        case Quadrant::VIII : {
            if(invalidIndex == GetNeighbor(modelIndex, Axis::PX)){
                modelIndex.x = invalidIndex;
            }
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NY)){
                modelIndex.y = invalidIndex;
            }
            else modelIndex.y -= 1;
            if(invalidIndex == GetNeighbor(modelIndex, Axis::NZ)){
                modelIndex.z = invalidIndex;
            }
            else modelIndex.z -= 1;
            break;
        }
        default : {
            modelIndex = {invalidIndex, invalidIndex, invalidIndex};
            break; 
        } 
    }
    return modelIndex;
}

ECAD_ALWAYS_INLINE bool EThermalNetworkExtraction::isBoundaryNode(size_t index) const
{
    auto [x, y, z] = GetModelIndex(index);
    if(x == 0 || x == (m_modelSize.x - 1)) return true;
    if(y == 0 || y == (m_modelSize.y - 1)) return true;
    if(z == 0 || z == (m_modelSize.z - 1)) return true;
    return false;
}

}//namespace esim
}//namespace ecad

#ifdef ECAD_HEADER_ONLY
#include "EThermalNetworkExtraction.cpp"
#endif

#endif//ECAD_ESIM_ETHERMALNETWOKREXTRACTION_H
