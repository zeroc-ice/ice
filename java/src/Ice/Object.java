// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class Object
{
    public
    Object()
    {
        _dispatcher = null;
    }

    public boolean
    equals(java.lang.Object rhs)
    {
        try
        {
            Object r = (Object)rhs;
            return this == r;
        }
        catch (ClassCastException ex)
        {
        }
        return false;
    }

    public int
    ice_hash()
    {
        return hashCode();
    }

    public static String[] __classIds =
    {
        "::Ice::Object"
    };

    public String[]
    __getClassIds()
    {
        return __classIds;
    }

    public boolean
    ice_isA(String s, Current current)
    {
        return s.equals("::Ice::Object");
    }

    public void
    ice_ping(Current current)
    {
        // Nothing to do.
    }

    public Dispatcher
    __dispatcher()
    {
        return new _ObjectDisp(this);
    }

    public final IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        synchronized (this)
        {
            if (_dispatcher == null)
            {
                _dispatcher = __dispatcher();
            }
        }

        return _dispatcher.__dispatch(in, current);
    }

    public void
    __write(IceInternal.BasicStream __os)
    {
        synchronized(_activeFacetMapMutex)
        {
            final int sz = _activeFacetMap.size();
            __os.writeInt(sz);

            java.util.Set set = _activeFacetMap.keySet();
            String[] keys = new String[sz];
            set.toArray(keys);
            for (int i = 0; i < sz; i++)
            {
                __os.writeString(keys[i]);
                __os.writeObject((Object)_activeFacetMap.get(keys[i]));
            }
        }
    }

    public void
    __read(IceInternal.BasicStream __is)
    {
        synchronized(_activeFacetMapMutex)
        {
            int sz = __is.readInt();

            _activeFacetMap.clear();

            Ice.ObjectHolder h = new Ice.ObjectHolder();
            while (sz-- > 0)
            {
                String key = __is.readString();
                __is.readObject("", h);
                _activeFacetMap.put(key, h.value);
            }
        }
    }

    public final void
    ice_addFacet(Object facet, String name)
    {
        synchronized(_activeFacetMapMutex)
        {
            _activeFacetMap.put(name, facet);
        }
    }

    public final void
    ice_removeFacet(String name)
    {
        synchronized(_activeFacetMapMutex)
        {
            _activeFacetMap.remove(name);
        }
    }

    public final void
    ice_removeAllFacets()
    {
        synchronized(_activeFacetMapMutex)
        {
            _activeFacetMap.clear();
        }
    }

    public final Object
    ice_findFacet(String name)
    {
        synchronized(_activeFacetMapMutex)
        {
            return (Object)_activeFacetMap.get(name);
        }
    }

    private java.util.HashMap _activeFacetMap = new java.util.HashMap();
    private java.lang.Object _activeFacetMapMutex = new java.lang.Object();
    private Dispatcher _dispatcher;
}
