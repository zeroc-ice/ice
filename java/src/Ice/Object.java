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

public abstract class Object
{
    public
    Object()
    {
    }

    public final int
    ice_hash()
    {
        // TODO: replace with hashCode? Call hashCode?
        return 0;
    }

    public boolean
    ice_isA(String s)
    {
        return s.equals("::Ice::Object");
    }

    public void
    ice_ping()
    {
        // Nothing to do
    }

    public final IceInternal.DispatchStatus
    ___ice_isA(IceInternal.Incoming __in)
    {
        Stream __is = __in.is();
        Stream __os = __in.os();
        String s = __is.readString();
        boolean __ret = ice_isA(s);
        __os.writeBool(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public final IceInternal.DispatchStatus
    ___ice_ping(IceInternal.Incoming __in)
    {
        ice_ping();
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public abstract String[]
    __getClassIds();

    public static String[] __all =
    {
        "ice_isA",
        "ice_ping"
    };

    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, String s)
    {
        int pos = java.util.Arrays.binarySearch(__all, s);
        if (pos < 0)
        {
            return IceInternal.DispatchStatus.DispatchOperationNotExist;
        }

        switch (pos)
        {
            case 0:
            {
                return ___ice_isA(in);
            }
            case 1:
            {
                return ___ice_ping(in);
            }
        }

        assert(false);
        return IceInternal.DispatchStatus.DispatchOperationNotExist;
    }

    public boolean
    __isMutating(String s)
    {
        //
        // None of the Ice::Object operations accessible via __dispatch()
        // is mutating.
        //
        return false;
    }

    public void
    __write(Stream __os)
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
    __read(Stream __is)
    {
        synchronized(_activeFacetMapMutex)
        {
            int sz = __is.readInt();

            _activeFacetMap.clear();

            while (sz-- > 0)
            {
                String key = __is.readString();
                Object value = __is.readObject("");
                _activeFacetMap.put(key, value);
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
