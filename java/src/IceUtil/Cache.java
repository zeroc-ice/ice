// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

//
// An abstraction to efficiently populate a Cache, without holding
// a lock while loading from a potentially slow store.
//

public class Cache
{
    public Cache(Store store)
    {
        _store = store;
    }
    
    public Object 
    getIfPinned(Object key)
    {
        synchronized(_map)
        {
            CacheValue val = (CacheValue)_map.get(key);
            return val == null ? null : val.obj;
        }
    }
    
    public Object
    unpin(Object key)
    {
        synchronized(_map)
        {
            CacheValue val = (CacheValue)_map.remove(key);
            return val == null ? null : val.obj;
        }
    }

    public void
    clear()
    {
        synchronized(_map)
        {
            _map.clear();
        }
    }
    
    public int
    size()
    {
        synchronized(_map)
        {
            return _map.size();
        }
    }

    //
    // Add an object to the cache without checking store
    // If there is already an object associated with this
    // key, the existing value remains in the map and is
    // returned.
    //
    public Object
    pin(Object key, Object o)
    {
        synchronized(_map)
        {
            CacheValue existingVal = (CacheValue)_map.put(key, new CacheValue(o));
            if(existingVal != null)
            {
                _map.put(key, existingVal);
                return existingVal.obj;
            }
            else
            {
                return null;
            }
        }
    }

    //
    // Return an object from the cache, loading it from the
    // store if necessary.
    //
    public Object
    pin(Object key)
    {
        return pinImpl(key, null);
    }

    //
    // Puts this key/value pair in the cache.
    // If this key is already in the cache or store, the given
    // key/value pair is not inserted and the existing value
    // is returned.
    //
    public Object
    putIfAbsent(Object key, Object newObj)
    {
        return pinImpl(key, newObj);
    }

    
    static private class CacheValue
    {
        CacheValue()
        {
        }

        CacheValue(Object obj)
        {
            this.obj = obj;
        }

        Object obj = null;
        CountDownLatch latch = null;
    }

    private Object
    pinImpl(Object key, Object newObj)
    {
        for(;;)
        {
            CacheValue val = null;
            CountDownLatch latch = null;
            
            synchronized(_map)
            {
                val = (CacheValue)_map.get(key); 
                if(val == null) 
                { 
                    val = new CacheValue(); 
                    _map.put(key, val); 
                } 
                else 
                { 
                    if(val.obj != null) 
                    { 
                        return val.obj;        
                    } 
                    if(val.latch == null) 
                    { 
                        // 
                        // The first queued thread creates the latch 
                        // 
                        val.latch = new CountDownLatch(1); 
                    } 
                    latch = val.latch; 
                } 
            }
            
            if(latch != null) 
            { 
                try
                {
                    latch.await();
                }
                catch(InterruptedException e)
                {
                    // Ignored
                }
                
                // 
                // val could be stale now, e.g. some other thread pinned and unpinned the 
                // object while we were waiting. 
                // So start over. 
                // 
                continue;
            } 
            else 
            {                     
                Object obj;
                try
                {
                    obj = _store.load(key);
                }
                catch(RuntimeException e)
                {
                    synchronized(_map) 
                    {
                        _map.remove(key);
                        latch = val.latch; 
                        val.latch = null; 
                    }
                    if(latch != null)  
                    { 
                        latch.countDown();
                        assert latch.getCount() == 0;
                    }
                    throw e;
                }
                
                synchronized(_map) 
                { 
                    if(obj != null) 
                    {  
                        val.obj = obj; 
                    } 
                    else 
                    { 
                        if(newObj == null)
                        {
                            //
                            // pin() did not find the object
                            //
                            
                            // 
                            // The waiting threads will have to call load() to see by themselves. 
                            // 
                            _map.remove(key);
                        }
                        else
                        {
                            //
                            // putIfAbsent() inserts key/newObj
                            //
                            val.obj = newObj;
                        }
                    } 
                    
                    latch = val.latch; 
                    val.latch = null; 
                } 
                if(latch != null)  
                { 
                    latch.countDown();
                    assert latch.getCount() == 0;
                } 
                return obj;
            }          
        }
    }


    private final java.util.Map _map = new java.util.HashMap();
    private final Store _store;

}
