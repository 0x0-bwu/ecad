#ifndef ECAD_ESIM_ETHERMALNETWOKREXTRACTION_H
#define ECAD_ESIM_ETHERMALNETWOKREXTRACTION_H
#include "thermal/model/ThermalNetwork.hpp"
#include "ECadCommon.h"
namespace ecad {
class ILayoutView;
namespace esim {

using namespace thermal::model;
class ECAD_API EThermalNetworkExtraction
{
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
    bool GenerateThermalNetwork(CPtr<ILayoutView> layout);
private:
    size_t GetFlattenIndex(const ModelIndex & index) const;
    ModelIndex GetModelIndex(size_t index) const;
    size_t GetNeighbor(size_t index, Axis axis) const;
    size_t GetNeighbor(ModelIndex modelIndex, Axis axis) const;
    ModelIndex GetMetalFractionBlockIndex(size_t index, Quadrant quadrant) const;//bot->up
    bool isBoundaryNode(size_t index) const;
    
private:
    ModelIndex m_modelSize;
    UPtr<ThermalNetwork> m_network;
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
