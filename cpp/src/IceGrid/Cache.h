//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_CACHE_H
#define ICE_GRID_CACHE_H

#include <IceGrid/Util.h>
#include <IceGrid/TraceLevels.h>

#include <mutex>

namespace IceGrid
{

class SynchronizationCallback
{
public:
    virtual void synchronized() = 0;
    virtual void synchronized(std::exception_ptr) = 0;
};

template<typename Key, typename Value>
class Cache
{
    using ValueType = std::shared_ptr<Value>;
    using ValueMap = std::map<Key, ValueType>;

public:

    Cache() : _entriesHint(_entries.end())
    {
    }

    virtual ~Cache() = default;

    bool
    has(const Key& key) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return getImpl(key) != nullptr;
    }

    void
    remove(const Key& key)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        removeImpl(key);
    }

    void
    setTraceLevels(const std::shared_ptr<TraceLevels>& traceLevels)
    {
        _traceLevels = traceLevels;
    }

    const std::shared_ptr<TraceLevels>& getTraceLevels() const { return _traceLevels; }

protected:

    virtual ValueType
    getImpl(const Key& key) const
    {
        auto p = const_cast<ValueMap&>(_entries).end();
        if(_entriesHint != p)
        {
            if(_entriesHint->first == key)
            {
                p = _entriesHint;
            }
        }

        if(p == const_cast<ValueMap&>(_entries).end())
        {
            p = const_cast<ValueMap&>(_entries).find(key);
        }

        if(p != const_cast<ValueMap&>(_entries).end())
        {
            const_cast<typename ValueMap::iterator&>(_entriesHint) = p;
            return p->second;
        }
        else
        {
            return nullptr;
        }
    }

    virtual ValueType
    addImpl(const Key& key, const ValueType& entry)
    {
        _entriesHint = _entries.insert(_entriesHint, { key, entry });
        return entry;
    }

    virtual void
    removeImpl(const Key& key)
    {
        auto p = _entries.end();
        if(_entriesHint != _entries.end())
        {
            if(_entriesHint->first == key)
            {
                p = _entriesHint;
            }
        }

        if(p == _entries.end())
        {
            p = _entries.find(key);
        }

        assert(p != _entries.end());
        if(p->second->canRemove())
        {
            _entries.erase(p);
            _entriesHint = _entries.end();
        }
        else
        {
            _entriesHint = p;
        }
    }

    std::shared_ptr<TraceLevels> _traceLevels;
    ValueMap _entries;
    typename ValueMap::iterator _entriesHint;

    mutable std::mutex _mutex;
    std::condition_variable _condVar;
};

template<typename T>
class CacheByString : public Cache<std::string, T>
{
public:

    std::vector<std::string>
    getAll(const std::string& expr)
    {
        std::lock_guard<std::mutex> lock(Cache<std::string, T>::_mutex);
        return getMatchingKeys<std::map<std::string, std::shared_ptr<T>>>(Cache<std::string, T>::_entries, expr);
    }
};

};

#endif
