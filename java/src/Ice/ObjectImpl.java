// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class ObjectImpl implements Object, java.lang.Cloneable
{
    public
    ObjectImpl()
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
        catch(ClassCastException ex)
        {
        }
        return false;
    }

    public java.lang.Object
    clone()
        throws java.lang.CloneNotSupportedException
    {
        //
        // Use super.clone() to perform a shallow copy of all members,
        // and then clone the facets manually.
        //
        ObjectImpl result = (ObjectImpl)super.clone();

        result._activeFacetMap = new java.util.HashMap();
        synchronized(_activeFacetMap)
        {
            if(!_activeFacetMap.isEmpty())
            {
                java.util.Iterator p = _activeFacetMap.entrySet().iterator();
                while(p.hasNext())
                {
                    java.util.Map.Entry e = (java.util.Map.Entry)p.next();
                    Object facet = (Object)e.getValue();
                    result._activeFacetMap.put(e.getKey(), facet.clone());
                }
            }
        }

        return result;
    }

    public int
    ice_hash()
    {
        return hashCode();
    }

    public static String[] __ids =
    {
        "::Ice::Object"
    };

    public boolean
    ice_isA(String s, Current current)
    {
        return s.equals(__ids[0]);
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

    public String[]
    ice_ids(Current current)
    {
        return __ids;
    }

    public static IceInternal.DispatchStatus
    ___ice_ids(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
        IceInternal.BasicStream __os = __in.os();
        String[] __ret = __obj.ice_ids(__current);
        __os.writeStringSeq(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public String
    ice_id(Current current)
    {
        return __ids[0];
    }

    public static IceInternal.DispatchStatus
    ___ice_id(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
        IceInternal.BasicStream __os = __in.os();
        String __ret = __obj.ice_id(__current);
        __os.writeString(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public final String[]
    ice_facets(Current current)
    {
        synchronized(_activeFacetMap)
        {
	    java.util.Set keySet = _activeFacetMap.keySet();
	    String[] v = new String[keySet.size()];
	    keySet.toArray(v);
	    return v;
	}
    }

    public static IceInternal.DispatchStatus
    ___ice_facets(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
        IceInternal.BasicStream __os = __in.os();
        String[] __ret = __obj.ice_facets(__current);
        __os.writeStringSeq(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public static String
    ice_staticId()
    {
        return __ids[0];
    }

    private static String[] __all =
    {
        "ice_facets",
        "ice_id",
        "ice_ids",
        "ice_isA",
        "ice_ping"
    };

    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        int pos = java.util.Arrays.binarySearch(__all, current.operation);
        if(pos < 0)
        {
            return IceInternal.DispatchStatus.DispatchOperationNotExist;
        }

        switch(pos)
        {
            case 0:
            {
                return ___ice_facets(this, in, current);
            }
            case 1:
            {
                return ___ice_id(this, in, current);
            }
            case 2:
            {
                return ___ice_ids(this, in, current);
            }
            case 3:
            {
                return ___ice_isA(this, in, current);
            }
            case 4:
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
        synchronized(_activeFacetMap)
        {
            final int sz = _activeFacetMap.size();
            __os.writeSize(sz);

            java.util.Set set = _activeFacetMap.keySet();
            String[] keys = new String[sz];
            set.toArray(keys);
            for(int i = 0; i < sz; i++)
            {
                __os.writeString(keys[i]);
                __os.writeObject((Object)_activeFacetMap.get(keys[i]));
            }
        }
    }

    public void
    __read(IceInternal.BasicStream __is)
    {
        synchronized(_activeFacetMap)
        {
            int sz = __is.readSize();

            _activeFacetMap.clear();

            while(sz-- > 0)
            {
                String key = __is.readString();
                Object value = __is.readObject("", null);
                _activeFacetMap.put(key, value);
            }
        }
    }

    public void
    __marshal(Ice.Stream __os)
    {
        synchronized(_activeFacetMap)
        {
            final int sz = _activeFacetMap.size();

            __os.startWriteDictionary("ice:facets", sz);
            java.util.Set set = _activeFacetMap.keySet();
            String[] keys = new String[sz];
            set.toArray(keys);
            for(int i = 0; i < sz; i++)
            {
                __os.startWriteDictionaryElement();
                __os.writeString("key", keys[i]);
                __os.writeObject("value", (Object)_activeFacetMap.get(keys[i]));
                __os.endWriteDictionaryElement();
            }
            __os.endWriteDictionary();
        }
    }

    public void
    __unmarshal(Ice.Stream __is)
    {
        synchronized(_activeFacetMap)
        {
            final String facetsName = "ice:facets";
            final String keyName = "key";
            final String valueName = "value";

            int sz = __is.startReadDictionary(facetsName);

            _activeFacetMap.clear();

            while(sz-- > 0)
            {
                __is.startReadDictionaryElement();
                String key = __is.readString(keyName);
                Object value = __is.readObject(valueName, "", null);
                _activeFacetMap.put(key, value);
                __is.endReadDictionaryElement();
            }
            __is.endReadDictionary();
        }
    }

    public final void
    ice_marshal(String name, Ice.Stream stream)
    {
        stream.writeObject(name, this);
    }

    public static Object
    ice_unmarshal(String name, Ice.Stream stream)
    {
        return stream.readObject(name, "", null);
    }

    public final void
    ice_addFacet(Object facet, String name)
    {
        synchronized(_activeFacetMap)
        {
            _activeFacetMap.put(name, facet);
        }
    }

    public final Object
    ice_removeFacet(String name)
    {
        synchronized(_activeFacetMap)
        {
            return (Object)_activeFacetMap.remove(name);
        }
    }

    public final void
    ice_removeAllFacets()
    {
        synchronized(_activeFacetMap)
        {
            _activeFacetMap.clear();
        }
    }

    public final Object
    ice_findFacet(String name)
    {
        synchronized(_activeFacetMap)
        {
            return (Object)_activeFacetMap.get(name);
	}
    }

    public final Object
    ice_findFacetPath(String[] path, int start)
    {
	int sz = path.length;
	
	if(start > sz)
	{
	    return null;
	}
	
	if(start == sz)
	{
	    return this;
	}

	Object f = ice_findFacet(path[start]);
	if(f != null)
	{
	    return f.ice_findFacetPath(path, start + 1);
	}
	else
	{
	    return f;
        }
    }

    private java.util.HashMap _activeFacetMap = new java.util.HashMap();
}
