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

    public static IceInternal.DispatchStatus
    ___ice_isA(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
        IceInternal.BasicStream __is = __in.is();
        IceInternal.BasicStream __os = __in.os();
        String __id = __is.readString();
        boolean __ret = __obj.ice_isA(__id, __current);
        __os.writeBool(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public void
    ice_ping(Current current)
    {
        // Nothing to do.
    }

    public static IceInternal.DispatchStatus
    ___ice_ping(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
        __obj.ice_ping(__current);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    private static String[] __all =
    {
        "ice_isA",
        "ice_ping"
    };

    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        int pos = java.util.Arrays.binarySearch(__all, current.operation);
        if (pos < 0)
        {
            return IceInternal.DispatchStatus.DispatchOperationNotExist;
        }

        switch (pos)
        {
            case 0:
            {
                return ___ice_isA(this, in, current);
            }
            case 1:
            {
                return ___ice_ping(this, in, current);
            }
        }

        assert(false);
        return IceInternal.DispatchStatus.DispatchOperationNotExist;
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
}
