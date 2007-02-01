// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_CACHE_H
#define ICE_UTIL_CACHE_H

#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/CountDownLatch.h>
#include <map>

namespace IceUtil
{

//
// An abstraction to efficiently populate a Cache, without holding
// a lock while loading from a database.
//

template<typename Key, typename Value>
class Cache
{
public:
    
    //
    // Latch and CacheValue are implementation details;
    // application code should not use them.
    //

    struct Latch : public CountDownLatch
    {
        Latch() :
            CountDownLatch(1),
            useCount(0)
        { 
        }
        int useCount;
    };

    struct CacheValue
    {
        CacheValue(const Handle<Value>& o) :
            obj(o),
            latch(0)
        {
        }
        
        Handle<Value> obj;
        Latch* latch;
    };

    typedef typename std::map<Key, CacheValue>::iterator Position;


    Handle<Value> getIfPinned(const Key&) const;

    void unpin(Position);
    bool unpin(const Key&);

    void clear();
    size_t size() const;

    bool pin(const Key&, const Handle<Value>&);
    
    Handle<Value> pin(const Key&);
    Handle<Value> putIfAbsent(const Key&, const Handle<Value>&);

protected:

    virtual Handle<Value> load(const Key&) = 0;

    virtual void pinned(const Handle<Value>&, Position)
    {
    }
    
    virtual ~Cache()
    {
    }

private:

    Handle<Value> pinImpl(const Key&, const Handle<Value>&);

    typedef std::map<Key, CacheValue> CacheMap;
    
    Mutex _mutex;
    CacheMap  _map;
};


template<typename Key, typename Value> Handle<Value> 
Cache<Key, Value>::getIfPinned(const Key& key) const
{
    Mutex::Lock sync(_mutex);
    typename CacheMap::const_iterator p = _map.find(key);
    if(p != _map.end())
    {
        return (*p).second.obj;
    }
    else
    {
        return 0;
    }
}

template<typename Key, typename Value> void
Cache<Key, Value>::unpin(typename Cache::Position p)
{
    Mutex::Lock sync(_mutex);
    _map.erase(p);
}

template<typename Key, typename Value> bool
Cache<Key, Value>::unpin(const Key& key)
{
    Mutex::Lock sync(_mutex);
    return _map.erase(key) > 0;
}

template<typename Key, typename Value> void 
Cache<Key, Value>::clear()
{
    Mutex::Lock sync(_mutex);
    _map.clear();
}

template<typename Key, typename Value> size_t 
Cache<Key, Value>::size() const
{
    Mutex::Lock sync(_mutex);
    return _map.size();
}
    
template<typename Key, typename Value> bool 
Cache<Key, Value>::pin(const Key& key, const Handle<Value>& obj)
{
    Mutex::Lock sync(_mutex);
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    std::pair<CacheMap::iterator, bool> ir = 
#else
    std::pair<typename CacheMap::iterator, bool> ir =
#endif 

#if defined(_MSC_VER)
       _map.insert(CacheMap::value_type(key, CacheValue(obj)));
#else
       _map.insert(typename CacheMap::value_type(key, CacheValue(obj)));
#endif       


    if(ir.second)
    {
        pinned(obj, ir.first);
    }
    return ir.second;
}

template<typename Key, typename Value> Handle<Value> 
Cache<Key, Value>::pin(const Key& key)
{
    return pinImpl(key, 0);
}

template<typename Key, typename Value> Handle<Value> 
Cache<Key, Value>::putIfAbsent(const Key& key, const Handle<Value>& obj)
{
    return pinImpl(key, obj);
}

template<typename Key, typename Value> Handle<Value> 
Cache<Key, Value>::pinImpl(const Key& key, const Handle<Value>& newObj)
{
    Latch* latch = 0;
    Position p;

    for(;;)
    {
        {
            Mutex::Lock sync(_mutex);
        
            //
            // Clean up latch from previous loop
            //
            if(latch != 0)
            {
                if(--latch->useCount == 0)
                {
                    delete latch;
                }
                latch = 0;
            }
    
#if defined(_MSC_VER) && (_MSC_VER < 1300)
            std::pair<CacheMap::iterator, bool> ir = 
#else
            std::pair<typename CacheMap::iterator, bool> ir =
#endif          
            
#if defined(_MSC_VER) || defined(__BCPLUSPLUS__)
                _map.insert(CacheMap::value_type(key, CacheValue(0)));
#else
                _map.insert(typename CacheMap::value_type(key, CacheValue(0)));
#endif    

            if(ir.second == false)
            {
                CacheValue& val = ir.first->second;
                if(val.obj != 0)
                {
                    return val.obj;
                }

                //
                // Otherwise wait
                //
                if(val.latch == 0)
                {
                    // 
                    // The first queued thread creates the latch 
                    // 
                    val.latch = new Latch; 
                }
                latch = val.latch;
                latch->useCount++;
            }
           
            p = ir.first;          
        }
                
        if(latch != 0) 
        { 
            latch->await();
            
            // 
            // p could be stale now, e.g. some other thread pinned and unpinned the 
            // object while we were waiting. 
            // So start over. 
            // 
            continue;
        } 
        else 
        {          
            Handle<Value> obj;
            try
            {
                obj = load(key);
            }
            catch(...)
            {
                {
                    Mutex::Lock sync(_mutex);
                    latch = p->second.latch;
                    p->second.latch = 0;
                    _map.erase(p);
                }
                if(latch != 0)  
                { 
                    assert(latch->getCount() == 1);
                    latch->countDown();
                }
                throw;
            }

            {
                Mutex::Lock sync(_mutex);

                latch = p->second.latch;
                p->second.latch = 0;

                try
                {
                    if(obj != 0) 
                    {  
                        p->second.obj = obj;
                        pinned(obj, p);
                    } 
                    else 
                    { 
                        if(newObj == 0)
                        {
                            //
                            // pin() did not find the object
                            //
                            
                            // 
                            // The waiting threads will have to call load() to see by themselves. 
                            // 
                            _map.erase(p);
                        }
                        else
                        {
                            //
                            // putIfAbsent() inserts key/newObj
                            //
                            p->second.obj = newObj;
                            pinned(newObj, p);
                        }
                    }
                }
                catch(...)
                {
                    if(latch != 0)  
                    { 
                        assert(latch->getCount() == 1);
                        latch->countDown();
                    }
                    throw;
                }
            }
            if(latch != 0)  
            { 
                assert(latch->getCount() == 1);
                latch->countDown();
            }
            return obj;
        }  
    }
}


}

#endif
