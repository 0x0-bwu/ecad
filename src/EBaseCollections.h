#pragma once
#include "interfaces/IIterator.h"
#include "ECollection.h"
#include "Protocol.h"
#include <unordered_map>
#include <type_traits>
#include <string>
namespace ecad {

template <typename Key, typename T,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator< std::pair<const Key, T> > >
class ECAD_API EUnorderedMapCollection : public ECollection 
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    
    template <typename Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollection);
        ar & boost::serialization::make_nvp("collection", m_collection);
    }

    template <typename Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollection);
        ar & boost::serialization::make_nvp("collection", m_collection);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
public:
    using CollectionObject = T;
    using CollectionContainer = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;
    EUnorderedMapCollection() : EUnorderedMapCollection(std::string{}) {}
    explicit EUnorderedMapCollection(std::string name) : ECollection(std::move(name)) {}
    virtual ~EUnorderedMapCollection() = default;

    ///Copy
    EUnorderedMapCollection(const EUnorderedMapCollection & other) { *this = other; }
    EUnorderedMapCollection & operator= (const EUnorderedMapCollection & other)
    {
        ECollection::operator=(other);
        m_collection.clear();
        for(const auto & object : other.m_collection){
            m_collection.insert(std::make_pair(object.first, CloneHelper(object.second)));
        }
        return *this;
    }

    T & operator[] (Key && key) { return m_collection[key]; }
    T & operator[] (const Key & key) { return m_collection[key]; }
    
    void Clear() { m_collection.clear(); }
    size_t Size() const { return m_collection.size(); }

    bool Count(const Key & key) const { return m_collection.count(key); }

    template <typename key_t, typename value_t>
    bool Insert(key_t && key, value_t && value)
    { 
        return m_collection.emplace(std::forward<key_t>(key), std::forward<value_t>(value)).second;
    }

    template <typename key_t>
    const T & At(key_t && key) const { return m_collection.at(std::forward<key_t>(key)); }

    CollectionContainer & Get() { return m_collection; }
    const CollectionContainer & Get() const { return m_collection; }

protected:
    CollectionContainer m_collection;
};

template <typename Key, typename T, typename std::enable_if<std::is_integral<Key>::value, bool>::type = true>
ECAD_ALWAYS_INLINE Key NextKey(const EUnorderedMapCollection<Key, T> & collection, const Key & key)
{
    Key next = key;
    while(collection.Count(next)){ next++; }
    return next;
}

// template <typename Key, typename T, typename std::enable_if<std::is_enum<Key>::value, bool>::type = true>
// ECAD_ALWAYS_INLINE Key NextKey(const EUnorderedMapCollection<Key, T> & collection, const Key & key)
// {
//     Key next = key;
//     while(collection.Count(next)){ next++; }
//     return next;
// }

template <typename T>
ECAD_ALWAYS_INLINE std::string NextKey(const EUnorderedMapCollection<std::string, T> & collection, const std::string & key)
{
    if(!collection.Count(key)) return key;
    size_t index = 1;
    while(true){
        std::string name = key + std::to_string(index);
        if(!collection.Count(name)) return name;
        index++;
    }
    return "";
}

template <typename Key, typename T,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<const Key, T> > >
class ECAD_API EUnorderedMapCollectionIterator : public IIterator<typename T::element_type>
{
    using Element = typename T::element_type;
    using Collection = EUnorderedMapCollection<Key, T, Hash, KeyEqual, Allocator>;
    using Iterator = typename Collection::CollectionContainer::const_iterator;
public:
    explicit EUnorderedMapCollectionIterator(const Collection & collection)
     : m_curr(collection.Get().begin())
     , m_end(collection.Get().end())
    {
    }

    virtual ~EUnorderedMapCollectionIterator() = default;

    Ptr<Element> Next()
    {
        if(m_curr == m_end) return nullptr;
        auto p = m_curr->second.get();
        ++m_curr;
        return p;
    }

    Ptr<Element> Current()
    {
        if(m_curr == m_end) return nullptr;
        return m_curr->second.get();
    }

    UPtr<IIterator<Element> > Clone() const
    {
        return UPtr<IIterator<Element> >(
            new EUnorderedMapCollectionIterator<Key, T, Hash, KeyEqual, Allocator>(*this)
            );
    }

private:
    Iterator m_curr;
    Iterator m_end;
};

template <typename Key, typename T,
          template <typename, typename> class SubContainer,
          template <typename> class SubAllocator = std::allocator,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<const Key, SubContainer<T, SubAllocator<T> > > > >
class ECAD_API EUnorderedMapComplexCollectionIterator : public IIterator<typename T::element_type>
{
    using Element = typename T::element_type;
    using Collection = EUnorderedMapCollection<Key, SubContainer<T, SubAllocator<T> >, Hash, KeyEqual, Allocator>;
    using Iterator1 = typename Collection::CollectionContainer::const_iterator;
    using Iterator2 = typename SubContainer<T, SubAllocator<T> >::const_iterator;
public:
    explicit EUnorderedMapComplexCollectionIterator(const Collection & collection)
     : m_curr1(collection.template Get().begin())
     , m_end1(collection.template Get().end())
    {
        UpdateIterator2();
    }

    virtual ~EUnorderedMapComplexCollectionIterator() = default;

    Ptr<Element> Next()
    {
        if(m_curr1 == m_end1) return nullptr;
        if(m_curr2 == m_end2) return nullptr;
        auto p = m_curr2->get();
        ++m_curr2;
        if(m_curr2 == m_end2){
            m_curr1++;
            UpdateIterator2();
        }
        return p;
    }

    Ptr<Element> Current()
    {
        if(m_curr1 == m_end1) return nullptr;
        if(m_curr2 == m_end2) return nullptr;
        return m_curr2->get();
    }

    UPtr<IIterator<Element> > Clone() const
    {
        return UPtr<IIterator<Element> >(
            new EUnorderedMapComplexCollectionIterator<Key, T, SubContainer, SubAllocator, Hash, KeyEqual, Allocator>(*this)
            );
    }

private:
    void UpdateIterator2()
    {
        if(m_curr1 != m_end1){
            m_curr2 = (m_curr1->second).begin();
            m_end2 = (m_curr1->second).end();
        }
    }

private:
    Iterator1 m_curr1;
    Iterator1 m_end1;
    Iterator2 m_curr2;
    Iterator2 m_end2;
};

template <typename T, typename Allocator = std::allocator<T> >
class ECAD_API EVectorCollection : public ECollection 
{
#ifdef ECAD_BOOST_SERIALIZATION_SUPPORT
    friend class boost::serialization::access;
    
    template <typename Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollection);
        ar & boost::serialization::make_nvp("collection", m_collection);
    }

    template <typename Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ECAD_UNUSED(version)
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ECollection);
        ar & boost::serialization::make_nvp("collection", m_collection);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif//ECAD_BOOST_SERIALIZATION_SUPPORT
public:
    using CollectionObject = T;
    using CollectionContainer = std::vector<T, Allocator>;
    EVectorCollection() : EVectorCollection(std::string{}) {}
    explicit EVectorCollection(std::string name) : ECollection(std::move(name)) {}
    virtual ~EVectorCollection() = default;

    ///Copy
    EVectorCollection(const EVectorCollection & other) { *this = other; }
    EVectorCollection & operator= (const EVectorCollection & other)
    {
        ECollection::operator=(other);
        m_collection.clear();
        for(const auto & object : other.m_collection){
            m_collection.push_back(CloneHelper(object));
        }
        return *this;
    }

    T & operator[] (size_t index) { return m_collection[index]; }
    const T & operator[] (size_t index) const { return m_collection[index]; }

    void Clear() { m_collection.clear(); }
    size_t Size() const { return m_collection.size(); }

    template <typename value_t>
    void Append(value_t && value) { m_collection.push_back(std::forward<value_t>(value)); }

    const T & At(size_t index) const { return m_collection.at(index); }
    const T & Front() const { return m_collection.front(); }
    const T & Back() const { return m_collection.back(); }
    T & Front() { return m_collection.front(); }
    T & Back() { return m_collection.back(); }

    CollectionContainer & Get() { return m_collection; }
    const CollectionContainer & Get() const { return m_collection; }

    template <typename Func>
    void Sort(Func && func) { std::sort(m_collection.begin(), m_collection.end(), std::forward<Func>(func)); }

protected:
    CollectionContainer m_collection;
};

template <typename T, typename Allocator = std::allocator<T> >
class ECAD_API EVectorCollectionIterator : public IIterator<typename T::element_type>
{
    using Element = typename T::element_type;
    using Collection = EVectorCollection<T, Allocator>;
    using Iterator = typename Collection::CollectionContainer::const_iterator;
public:
    explicit EVectorCollectionIterator(const Collection & collection)
     : m_curr(collection.Get().begin())
     , m_end(collection.Get().end())
    {
    }

    virtual ~EVectorCollectionIterator() = default;

    Ptr<Element> Next()
    {
        if(m_curr == m_end) return nullptr;
        auto p = m_curr->get();
        ++m_curr;
        return p;
    }

    Ptr<Element> Current()
    {
        if(m_curr == m_end) return nullptr;
        return m_curr->get();
    }

    UPtr<IIterator<Element> > Clone() const
    {
        return UPtr<IIterator<Element> >(new EVectorCollectionIterator<T, Allocator>(*this));
    }
private:
    Iterator m_curr;
    Iterator m_end;
};

}//namespace ecad