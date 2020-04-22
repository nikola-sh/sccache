#pragma once

#define BOOST_USE_WINDOWS_H

#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/unordered_map.hpp>

#include <boost/functional/hash.hpp>
#include <functional>

/*
int main ()
{
    using namespace boost::interprocess;
    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove() { shared_memory_object::remove("MySharedMemory"); }
        ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
    } remover;

    //Create shared memory
    managed_shared_memory segment(create_only, "MySharedMemory", 65536);

    //Note that unordered_map<Key, MappedType>'s value_type is std::pair<const Key, MappedType>,
    //so the allocator must allocate that pair.
    typedef int    KeyType;
    typedef float  MappedType;
    typedef std::pair<const int, float> ValueType;

    //Typedef the allocator
    typedef allocator<ValueType, managed_shared_memory::segment_manager> ShmemAllocator;

    //Alias an unordered_map of ints that uses the previous STL-like allocator.
    typedef boost::unordered_map
        < KeyType               , MappedType
        , boost::hash<KeyType>  ,std::equal_to<KeyType>
        , ShmemAllocator>
        MyHashMap;

    //Construct a shared memory hash map.
    //Note that the first parameter is the initial bucket count and
    //after that, the hash function, the equality function and the allocator
    MyHashMap *myhashmap = segment.construct<MyHashMap>("MyHashMap")  //object name
        ( 3, boost::hash<int>(), std::equal_to<int>()                  //
            , segment.get_allocator<ValueType>());                         //allocator instance

                                                                           //Insert data in the hash map
    for(int i = 0; i < 100; ++i){
        myhashmap->insert(ValueType(i, (float)i));
    }
    return 0;
}*/

namespace bi = boost::interprocess;

struct KeyType
{
    KeyType()
    {
        std::memset(path, 0, sizeof(path));
    }

    bool operator==(KeyType const& other) const
    {
        return wmemcmp(path, other.path, _countof(path)) == 0;
    }

    wchar_t path[MAX_PATH];
};

inline std::size_t hash_value(KeyType const& key)
{
    return boost::hash_value(key.path);
}

class SharedCache
{
private:
    
    typedef DWORD MappedType;
    typedef std::pair<KeyType, MappedType> ValueType;

    typedef bi::allocator<ValueType, bi::managed_shared_memory::segment_manager> ShmemAllocator;

    typedef boost::unordered_map
        < KeyType               , MappedType
        , boost::hash<KeyType>  ,std::equal_to<KeyType>
        , ShmemAllocator>
        HashMap;

public:
    SharedCache(bool clear = false)
    {
        std::unique_ptr<bi::named_mutex> mutex(new bi::named_mutex(bi::open_or_create, "C39A3123-7DF9-47EA-9226-C79737EC9C9C_mutex"));

        if (clear)
            bi::shared_memory_object::remove("C39A3123-7DF9-47EA-9226-C79737EC9C9C");

        bi::scoped_lock<bi::named_mutex> lock(*mutex);
        std::unique_ptr<bi::managed_shared_memory> segment(new bi::managed_shared_memory(bi::open_or_create, "C39A3123-7DF9-47EA-9226-C79737EC9C9C", 10 * 1024 * 1024));

        HashMap *myhashmap = segment->find_or_construct<HashMap>("SharedCache")(1000, boost::hash<KeyType>(), std::equal_to<KeyType>(), segment->get_allocator<ValueType>());

        m_segment = std::move(segment);
        m_mutex = std::move(mutex);
        m_cache = myhashmap;
    }

    void Add(wchar_t const* path, DWORD err)
    {
        if (wcslen(path) >= MAX_PATH)
            return;

        ValueType val;
        wcscpy(val.first.path, path);
        val.second = err;

        bi::scoped_lock<bi::named_mutex> lock(*m_mutex);
        m_cache->insert(val);
    }

    bool Check(wchar_t const* path)
    {
        if (wcslen(path) >= MAX_PATH)
            return false;

        KeyType key;
        wcscpy(key.path, path);

        bi::scoped_lock<bi::named_mutex> lock(*m_mutex);

        auto it = m_cache->find(key);
        if (it == m_cache->end())
            return false;

        return true;
    }

private:
    std::unique_ptr<bi::managed_shared_memory> m_segment;
    std::unique_ptr<bi::named_mutex> m_mutex;
    HashMap* m_cache = nullptr;
};
