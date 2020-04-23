#pragma once

#define BOOST_USE_WINDOWS_H

#include <boost/interprocess/managed_windows_shared_memory.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_sharable_mutex.hpp>
#include <boost/unordered_map.hpp>

#include <boost/functional/hash.hpp>
#include <functional>

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
    SharedCache(bool create = false)
    {
        static const char* const SharedMemoryName = "C39A3123-7DF9-47EA-9226-C79737EC9C9C";
        static const char* const SharedMemoryMutexName = "C39A3123-7DF9-47EA-9226-C79737EC9C9C_mutex";

        if (create)
            bi::named_sharable_mutex::remove(SharedMemoryMutexName);

        std::unique_ptr<bi::named_sharable_mutex> mutex(
            create
            ? new bi::named_sharable_mutex(bi::create_only, SharedMemoryMutexName)
            : new bi::named_sharable_mutex(bi::open_only, SharedMemoryMutexName)
        );

        bi::scoped_lock<bi::named_sharable_mutex> lock(*mutex);

        std::unique_ptr<bi::managed_windows_shared_memory> segment(
            create
            ? new bi::managed_windows_shared_memory(bi::create_only, SharedMemoryName, 200 * 1024 * 1024)
            : new bi::managed_windows_shared_memory(bi::open_only, SharedMemoryName)
        );

        HashMap *myhashmap = segment->find_or_construct<HashMap>("SharedCache")(100000, boost::hash<KeyType>(), std::equal_to<KeyType>(), segment->get_allocator<ValueType>());

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

        bi::scoped_lock<bi::named_sharable_mutex> lock(*m_mutex);
        m_cache->insert(val);
    }

    bool Get(wchar_t const* path, DWORD& err)
    {
        if (wcslen(path) >= MAX_PATH)
            return false;

        KeyType key;
        wcscpy(key.path, path);

        bi::sharable_lock<bi::named_sharable_mutex> lock(*m_mutex);

        auto it = m_cache->find(key);
        if (it == m_cache->end())
            return false;

        err = it->second;
        return true;
    }

private:
    std::unique_ptr<bi::managed_windows_shared_memory> m_segment;
    std::unique_ptr<bi::named_sharable_mutex> m_mutex;
    HashMap* m_cache = nullptr;
};
