// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

/**
 * An abstraction to efficiently maintain a cache, without holding
 * a lock on the entire cache while objects are being loaded from
 * their backing store. This class is useful mainly to implement
 * evictors, such as used by Freeze.
 *
 * @see Store
 **/

public class Cache
{
    /**
     * Initialize a cache using the specified backing store.
     **/
    public Cache(Store store)
    {
        _store = store;
    }

    /**
     * Return the value stored for the given key from the cache.
     *
     * @param key The key for the object to look up in the cache.
     *
     * @return If the cache contains an entry for the key, the return value
     * is the object corresponding to the key; otherwise, the return value
     * is null. <code>getIfPinned</code> does not call {@link Store#load}.
     *
     * @see Store#load
     **/
    public Object
    getIfPinned(Object key)
    {
        synchronized(_map)
        {
            CacheValue val = _map.get(key);
            return val == null ? null : val.obj;
        }
    }

    /**
     * Removes the entry for the given key from the cache.
     *
     * @param key The key for the entry to remove.
     *
     * @return If the cache contains an entry for the key, the
     * return value is the corresponding object; otherwise, the
     * return value is <code>null</code>.
     **/
    public Object
    unpin(Object key)
    {
        synchronized(_map)
        {
            CacheValue val = _map.remove(key);
            return val == null ? null : val.obj;
        }
    }

    /**
     * Removes all entries from the cache.
     **/
    public void
    clear()
    {
        synchronized(_map)
        {
            _map.clear();
        }
    }

    /**
     * Returns the number of entries in the cache.
     *
     * @return The number of entries.
     **/
    public int
    size()
    {
        synchronized(_map)
        {
            return _map.size();
        }
    }

    /**
     * Adds a key-value pair to the cache.
     * This version of <code>pin</code> does not call {@link Store#load} to retrieve
     * an entry from backing store if an entry for the given key is not yet in the cache. This
     * is useful to add a newly-created object to the cache.
     *
     * @param key The key for the entry.
     * @param o The value for the entry.
     * @return If the cache already contains an entry with the given key, the entry is
     * unchanged and <code>pin(Object, Object)</code> returns the original value for the entry; otherwise,
     * the entry is added and <code>pin(Object, Object)</code> returns <code>null</code>.
     **/
    public Object
    pin(Object key, Object o)
    {
        synchronized(_map)
        {
            CacheValue existingVal = _map.put(key, new CacheValue(o));
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

    /**
     * Returns an object from the cache.
     * If no entry with the given key is in the cache, <code>pin</code> calls
     * {@link Store#load} to retrieve the corresponding value (if any) from the
     * backing store.
     *
     * @param key The key for the entry to retrieve.
     * @return Returns the value for the corresponding key if the cache
     * contains an entry for the key. Otherwise, <code>pin(Object)</code> calls
     * {@link Store#load} and the return value is whatever is returned by
     * <code>load</code>; if <code>load</code> throws an exception, that exception
     * is thrown by <code>pin(Object)</code>.
     **/
    public Object
    pin(Object key)
    {
        return pinImpl(key, null);
    }

    /**
     * Adds a key-value pair to the cache.
     * @param key The key for the entry.
     * @param newObj The value for the entry.
     * @return If the cache already contains an entry for the given key,
     * <code>putIfAbsent</code> returns the original value for that key.
     * If no entry is for the given key is in the cache, <code>putIfAbsent</code>
     * calls {@link Store#load} to retrieve the corresponding entry (if any) from
     * the backings store and returns the value returned by <code>load</code>.
     * If the cache does not contain an entry for the given key and <code>load</code>
     * does not return a value for the key, <code>putIfAbsent</code> adds the new entry
     * and returns <code>null</code>.
     **/
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
        java.util.concurrent.CountDownLatch latch = null;
    }

    private Object
    pinImpl(Object key, Object newObj)
    {
        for(;;)
        {
            CacheValue val = null;
            java.util.concurrent.CountDownLatch latch = null;

            synchronized(_map)
            {
                val = _map.get(key);
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
                        val.latch = new java.util.concurrent.CountDownLatch(1);
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

    private final java.util.Map<Object, CacheValue> _map = new java.util.HashMap<Object, CacheValue>();
    private final Store _store;
}
