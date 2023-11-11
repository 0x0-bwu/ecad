#pragma once

#include "generic/math/MathUtility.hpp"
#include "generic/tools/Format.hpp"
#include "generic/circuit/MNA.hpp"
#include <unordered_map>
#include <memory>
#include <queue>
#include <map>

namespace thermal {
namespace model {

template <typename num_type>
class ThermalNetworkMatrixBuilder;

template <typename num_type>
class ThermalNetwork
{
    friend class ThermalNetworkMatrixBuilder<num_type>;
public:
    inline static constexpr num_type unknownT = std::numeric_limits<num_type>::max();
    struct Node
    {
        num_type t = unknownT;//unit: K
        num_type c = 0;
        num_type hf = 0;//unit: W
        num_type htc = 0;//unit: W/k
        std::vector<size_t> ns;
        std::vector<num_type> rs;

        std::string msg(size_t index) const
        {
            using namespace generic::fmt;
            std::stringstream ss;
            ss << Fmt2Str("ID: %1%, T:%2%, C:%3%, HF:%4%, HTC:%5%", index, t, c, hf, htc);
            if (not ns.empty()) {
                ss << ", N:[";
                for (size_t i = 0; i < ns.size(); ++i)
                    ss << Fmt2Str("%1%(%2%) ", ns.at(i), rs.at(i));
                ss << ']';
            }
            return ss.str();
        }
    };

    struct Edge
    {
        size_t x, y;
        num_type  r;//unit: K/W
    };

    explicit ThermalNetwork(size_t nodes)
    {
        m_nodes.assign(nodes, Node{});
    }

    virtual ~ThermalNetwork() = default;

    size_t Size() const
    {
        return m_nodes.size();
    }

    size_t Source() const
    {
        size_t size{0};
        for (const auto & node : m_nodes) {
            if (node.hf != 0 || node.htc != 0)
                size++;
        }
        return size;//todo , remove
    }

    Node & operator[] (size_t i) 
    {
        return m_nodes[i]; 
    }

    const Node & operator[] (size_t i) const
    {
        return m_nodes.at(i);
    }
    
    size_t AppendNode(num_type t = unknownT)
    {
        size_t index = Size();
        m_nodes.push_back(Node());
        m_nodes.back().t = t;
        return index;
    }

    const std::vector<size_t> & NS(size_t node) const
    {
        return m_nodes[node].ns;
    }

    void SetT(size_t node, num_type t)
    {
        m_nodes[node].t = t;
    }

    num_type GetT(size_t node) const
    {
        return m_nodes[node].t;
    }

    void SetHF(size_t node, num_type hf)
    {
        m_nodes[node].hf = hf;
    }

    num_type GetHF(size_t node) const
    {
        return m_nodes[node].hf;
    }

    void SetHTC(size_t node, num_type htc)
    {
        m_nodes[node].htc = htc;
    }

    num_type GetHTC(size_t node) const
    {
        return m_nodes[node].htc;
    }

    void SetC(size_t node, num_type c)
    {
        m_nodes[node].c = c;
    }

    void SetR(size_t node1, size_t node2, num_type r)
    {
        //todo eff
        for (size_t i = 0; i < m_nodes[node1].ns.size(); ++i) {
            if (m_nodes[node1].ns[i] == node2) {
                r = 1 / (1 / r + 1 / m_nodes[node1].rs[i]);
                m_nodes[node1].rs[i] = r;
                for (size_t j = 0; j < m_nodes[node2].ns.size(); ++j) {
                    if (m_nodes[node2].ns[j] == node1)
                        m_nodes[node2].rs[j] = r;
                }
                return;
            }
        }
        m_nodes[node1].ns.push_back(node2);
        m_nodes[node1].rs.push_back(r);

        m_nodes[node2].ns.push_back(node1);//wbtest remove
        m_nodes[node2].rs.push_back(r);//wbtest remove
    }

    std::vector<Node> & GetNodes()
    {
        return m_nodes;
    }
    
    const std::vector<Node> & GetNodes() const
    {
        return m_nodes;
    }

    num_type TotalHF() const
    {
        num_type total{0};
        for (const auto & node : m_nodes)
            total += node.hf;
        return total;
    }

private:
    std::vector<Node> m_nodes;
};

template <typename num_type>
class ThermalNetworkMatrixBuilder
{
    using Edge = typename ThermalNetwork<num_type>::Edge;
public:
    explicit ThermalNetworkMatrixBuilder(const ThermalNetwork<num_type> & network)
     : m_network(network) { BuildMatrixIndicesMap(); }
    
    size_t GetMatrixSize() const
    {
        return m_mnMap.size();
    }

    num_type GetDiagCoeff(size_t mIndex) const
    {
        size_t nIndex = m_mnMap.at(mIndex);
        return GetDiagCoeffInternal(nIndex);
    }

    num_type GetCoeff(size_t mRow, size_t mCol) const
    {
        size_t nRow = m_mnMap.at(mRow);
        size_t nCol = m_mnMap.at(mCol);
        return GetCoeffInternal(nRow, nCol);
    }

    num_type GetRhs(size_t mIndex, const num_type & refT) const
    {
        size_t nIndex = m_mnMap.at(mIndex);
        return GetRhsInternal(nIndex, refT);
    }

    void GetCoeffs(std::list<Edge> & edges) const
    {
        edges.clear();
        auto mSize = GetMatrixSize();
        auto mark = std::unique_ptr<std::vector<bool> >(new std::vector<bool>(mSize, false));
        GetCoeffsBFS(m_mnMap.begin()->first, *mark, edges);
    }
    
    void GetDiagCoeffs(std::vector<num_type> & diagCoeffs) const
    {
        auto mSize = GetMatrixSize();
        diagCoeffs.resize(mSize);
        for(size_t mIdx = 0; mIdx < mSize; ++mIdx)
            diagCoeffs[mIdx] = GetDiagCoeff(mIdx);
    }

    const std::unordered_map<size_t, size_t> & GetMatrixNodeIndicesMap() const
    {
        return m_mnMap;
    }

    const std::unordered_map<size_t, size_t> & GetNodeMatrixIndicesMap() const
    {
        return m_nmMap;
    }

private:
    void BuildMatrixIndicesMap()
    {
        size_t index = 0;
        for(size_t i = 0; i < m_network.m_nodes.size(); ++i) {
            const auto & node = m_network.m_nodes.at(i);
            if(node.t != ThermalNetwork<num_type>::unknownT) continue;
            m_mnMap.insert(std::make_pair(index, i));
            m_nmMap.insert(std::make_pair(i, index));
            index++;
        }
    }

    num_type GetDiagCoeffInternal(size_t nIndex) const
    {
        num_type coeff = 0;
        const auto & nodes = m_network.m_nodes;
        for(const auto & r : nodes[nIndex].rs)
            coeff += 1 / r;
        coeff += nodes[nIndex].htc;
        return coeff;
    }

    num_type GetCoeffInternal(size_t nRow, size_t nCol) const
    {
        const auto & nodes = m_network.m_nodes;
        const auto & ns = nodes[nRow].ns;
        for(size_t i = 0; i < ns.size(); ++i){
            if(ns[i] == nCol) return -1 / (nodes[nRow].rs[i]);
        }
        return 0;
    }

    num_type GetRhsInternal(size_t nIndex, const num_type & refT) const
    {
        const auto & nodes = m_network.m_nodes;
        num_type rhs = nodes[nIndex].hf + nodes[nIndex].htc * refT;
        const auto & ns = nodes[nIndex].ns;
        for(auto n : ns) {
            if(nodes[n].t != ThermalNetwork<num_type>::unknownT)
                rhs -= GetCoeffInternal(nIndex, n) * nodes[n].t;
        }
        return rhs;
    }

    void GetCoeffsBFS(size_t mIndex, std::vector<bool> & mark, std::list<Edge> & edges) const
    {
        const auto & nodes = m_network.m_nodes;
        auto queue = std::unique_ptr<std::queue<size_t> >(new std::queue<size_t>{});
        mark[mIndex] = true;
        queue->push(mIndex);
        while(!(queue->empty())){
            auto mv = queue->front();
            auto nv = m_mnMap.at(mv);
            queue->pop();
            const auto & ns = nodes[nv].ns;
            const auto & rs = nodes[nv].rs;
            for(size_t i = 0; i < ns.size(); ++i){
                auto nw = ns[i];
                if(nodes[nw].t != ThermalNetwork<num_type>::unknownT) continue;
                auto mw = m_nmMap.at(ns[i]);
                edges.push_back(Edge{mv, mw, -1 / rs[i]});
                if(!mark[mw]){
                    mark[mw] = true;
                    queue->push(mw);
                }
            }
        }
    }

private:
    std::unordered_map<size_t, size_t> m_mnMap;//matrix index->node index;
    std::unordered_map<size_t, size_t> m_nmMap;//node index->matrix index;
    const ThermalNetwork<num_type> & m_network;
};

using namespace generic::ckt;

template <typename num_type>
inline DenseVector<num_type> makeRhs(const ThermalNetwork<num_type> & network, num_type refT)
{
    const size_t nodes = network.Size();
    const size_t source = network.Source();
    DenseVector<num_type> rhs(source);
    for(size_t i = 0, s = 0; i < nodes; ++i) {
        const auto & node = network[i];
        if (node.hf != 0 || node.htc != 0)
            rhs[s++] = node.hf + node.htc * refT;
    }
    return rhs;
}

template <typename num_type>
inline MNA<SparseMatrix<num_type> > makeMNA(const ThermalNetwork<num_type> & network)
{
    using Matrix = SparseMatrix<num_type>;
    using Triplets = std::vector<Eigen::Triplet<num_type> >;
    
    MNA<Matrix> m;
    const size_t nodes = network.Size();
    const size_t source = network.Source();
    m.G = Matrix(nodes, nodes);
    m.C = Matrix(nodes, nodes);
    m.L = Matrix(nodes, nodes);
    m.B = Matrix(nodes, source);

    Triplets tG, tC, tL, tB;
    for (size_t i = 0, s = 0; i < nodes; ++i) {
        const auto & node = network[i];
        for (size_t j = 0; j < node.ns.size(); ++j) {
            if (auto n = node.ns.at(j); n > i) { //todo, remove ">"" check after modify to single edage 
                if (auto r = node.rs.at(j); r > 0)
                    mna::Stamp(tG, i, n, num_type{1} / r);
            }
        }
        if (node.htc != 0) mna::Stamp(tG, i, node.htc);
        if (node.c > 0) mna::Stamp(tC, i, node.c);
        if (node.hf != 0 || node.htc != 0)
            tB.emplace_back(i, s++, 1);
        tL.emplace_back(i, i, 1);//todo identity mat
    }
    m.G.setFromTriplets(tG.begin(), tG.end());
    m.C.setFromTriplets(tC.begin(), tC.end());
    m.B.setFromTriplets(tB.begin(), tB.end());
    m.L.setFromTriplets(tL.begin(), tL.end());
    return m;
}

}//namespace model
}//namespace thermal