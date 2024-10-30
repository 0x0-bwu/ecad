#pragma once
#include "ECadCommon.h"
#include "Protocol.h"
namespace ecad {

//todo refactor
template <typename Left, typename Right,
          typename LeftHash = std::hash<Left>, typename RightHash = std::hash<Right>>
class EBidirectionMap
{
public:
    std::unordered_map<Left, Right> forward;
    std::unordered_map<Right, Left> backward;
    EBidirectionMap() = default;
    
    void Insert(Left left, Right right)
    {
        forward.emplace(left, right);
        backward.emplace(right, left);
    }

    bool hasLeft(const Left & left) const
    {
        return forward.count(left);
    }

    bool hasRight(const Right & right) const
    {
        return backward.count(right);
    }

    Left & GetLeft(const Right & right)
    {
        return backward[right];
    }

    const Left & GetLeft(const Right & right) const
    {
        return backward.at(right);
    }

    Right & GetRight(const Left & left)
    {
        return forward[left];
    }

    const Right & GetRight(const Left & left) const
    {
        return forward.at(left);
    }

    void Clear()
    {
        forward.clear();
        backward.clear();
    }
};

//todo refactor
class ELookupTable1D : public Printable
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & boost::serialization::make_nvp("data", m_data);
    }

#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
    using DataType = std::map<EFloat, EFloat>;
public:
    ELookupTable1D() = default;
    virtual ~ELookupTable1D() = default;
    void AddSample(EFloat key, EFloat value);
    EFloat Lookup(EFloat key) const;
    bool Empty() const;

    typename DataType::iterator begin() { return m_data.begin(); }
    typename DataType::iterator end() { return m_data.end(); }
    typename DataType::const_iterator cbegin() { return m_data.cbegin(); }
    typename DataType::const_iterator cend() { return m_data.cend(); }

protected:
    virtual void PrintImp(std::ostream & os) const override;

protected:
    DataType m_data;
};

ECAD_ALWAYS_INLINE void ELookupTable1D::AddSample(EFloat key, EFloat value)
{
    m_data.emplace(key, value);
}

ECAD_ALWAYS_INLINE EFloat ELookupTable1D::Lookup(EFloat key) const
{
    if (m_data.empty()) return 0;
    if (m_data.size() == 1) return m_data.begin()->second;
    if (key < m_data.begin()->first) return m_data.begin()->second;
    if (key > m_data.rbegin()->first) return m_data.rbegin()->second;
    auto h = m_data.lower_bound(key); if (h == m_data.begin()) return h->second;
    auto l = h; l--;
    return l->second + (key - l->first) * (h->second - l->second) / (h->first - l->first);
}

ECAD_ALWAYS_INLINE bool ELookupTable1D::Empty() const
{
    return m_data.empty();
}

ECAD_ALWAYS_INLINE void ELookupTable1D::PrintImp(std::ostream & os) const
{
    os << '{';
    for (auto [key, value] : m_data)
        os << '{' << key << ',' << value << "},";
    os << '}' << ECAD_EOL;
}

} // namespace ecad
