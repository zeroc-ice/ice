// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************
namespace Ice
{
	
using System.Collections;
using System.Diagnostics;

public class ObjectImpl : Object, System.ICloneable
{
    public
    ObjectImpl()
    {
	_activeFacetMap = new Hashtable();
    }

    public virtual int
    CompareTo(object other)
    {
	if(other == null)
	{
	    return 1;
	}
	if(!(other is Ice.Object))
	{
	    throw new System.ArgumentException("expected object of type Ice.Object", "other");
	}
	int thisHash = GetHashCode();
	int otherHash = other.GetHashCode();
	return thisHash < otherHash ? -1 : (thisHash > otherHash ? 1 : 0);
    }

    public object
    Clone()
    {
	//
	// Use base Clone() to perform a shallow copy of all members,
	// and then clone the facets manually.
	//
	ObjectImpl result = (ObjectImpl) base.MemberwiseClone();
	
	result._activeFacetMap = new Hashtable();
	lock(_activeFacetMap)
	{
	    foreach(DictionaryEntry e in _activeFacetMap)
	    {
	        result._activeFacetMap[e.Key] = ((Object)e.Key).Clone();
	    }
	}
	
	return result;
    }
    
    public virtual int
    ice_hash()
    {
	return GetHashCode();
    }
    
    public static readonly string[] __ids = new string[] { "::Ice::Object" };

    private static readonly StringSeq __idSeq = new StringSeq(__ids);
    
    public virtual bool
    ice_isA(string s, Current current)
    {
	return s.Equals(__ids[0]);
    }
    
    public static IceInternal.DispatchStatus
    ___ice_isA(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
	IceInternal.BasicStream __is = __in.istr();
	IceInternal.BasicStream __os = __in.ostr();
	string __id = __is.readString();
	bool __ret = __obj.ice_isA(__id, __current);
	__os.writeBool(__ret);
	return IceInternal.DispatchStatus.DispatchOK;
    }
    
    public virtual void
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
    
    public virtual StringSeq
    ice_ids(Current current)
    {
	return __idSeq;
    }
    
    public static IceInternal.DispatchStatus
    ___ice_ids(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
	IceInternal.BasicStream __os = __in.ostr();
	__os.writeStringSeq(__obj.ice_ids(__current));
	return IceInternal.DispatchStatus.DispatchOK;
    }
    
    public virtual string
    ice_id(Current current)
    {
	return __ids[0];
    }
    
    public static IceInternal.DispatchStatus
    ___ice_id(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
	IceInternal.BasicStream __os = __in.ostr();
	string __ret = __obj.ice_id(__current);
	__os.writeString(__ret);
	return IceInternal.DispatchStatus.DispatchOK;
    }
    
    public FacetPath
    ice_facets(Current current)
    {
	lock(_activeFacetMap)
	{	    
	    FacetPath fp = new FacetPath();
	    foreach(string path in _activeFacetMap.Keys)
	    {
		fp.Add(path);
	    }
	    return fp;
	}
    }
    
    public static IceInternal.DispatchStatus
    ___ice_facets(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
	IceInternal.BasicStream __os = __in.ostr();
	__os.writeStringSeq(__obj.ice_facets(__current));
	return IceInternal.DispatchStatus.DispatchOK;
    }
    
    public static string ice_staticId()
    {
	return __ids[0];
    }
    
    private static readonly string[] __all = new string[]
    {
	"ice_facets", "ice_id", "ice_ids", "ice_isA", "ice_ping"
    };
    
    public virtual IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming inc, Current current)
    {
	int pos = System.Array.BinarySearch(__all, current.operation);
	if(pos < 0)
	{
	    return IceInternal.DispatchStatus.DispatchOperationNotExist;
	}
	
	switch (pos)
	{
	    case 0: 
	    {
		    return ___ice_facets(this, inc, current);
	    }
	    case 1: 
	    {
		    return ___ice_id(this, inc, current);
	    }
	    case 2: 
	    {
		    return ___ice_ids(this, inc, current);
	    }
	    case 3: 
	    {
		    return ___ice_isA(this, inc, current);
	    }
	    case 4: 
	    {
		    return ___ice_ping(this, inc, current);
	    }
	}
	
	Debug.Assert(false);
	return IceInternal.DispatchStatus.DispatchOperationNotExist;
    }
    
    public virtual void
    __write(IceInternal.BasicStream __os, bool __marshalFacets)
    {
	__os.writeTypeId(ice_staticId());
	__os.startWriteSlice();
	
	if(__marshalFacets)
	{
	    lock(_activeFacetMap)
	    {
		__os.writeSize(_activeFacetMap.Count);
		foreach(DictionaryEntry entry in _activeFacetMap)
		{
		    __os.writeString((string)entry.Key);
		    __os.writeObject((Object)entry.Value);
		}
	    }
	}
	else
	{
	    __os.writeSize(0);
	}
	
	__os.endWriteSlice();
    }
    
    private class Patcher : IceInternal.Patcher
    {
	internal
	Patcher(ObjectImpl enclosingInstance, string key)
	{
	    _enclosingInstance = enclosingInstance;
	    __key = key;
	}
	
	public virtual void
	patch(Ice.Object v)
	{
	    _enclosingInstance._activeFacetMap[__key] = v;
	}
	
	public virtual string
	type()
	{
	    return Ice.ObjectImpl.ice_staticId();
	}
	
	private ObjectImpl _enclosingInstance;
	private string __key;
    }
    
    public virtual void
    __read(IceInternal.BasicStream __is, bool __rid)
    {
	lock(_activeFacetMap)
	{
	    if(__rid)
	    {
		string myId = __is.readTypeId();
	    }
	    
	    __is.startReadSlice();
	    
	    int sz = __is.readSize();
	    
	    _activeFacetMap.Clear();
	    
	    while(sz-- > 0)
	    {
		string key = __is.readString();
		__is.readObject(new Patcher(this, key));
	    }
	    
	    __is.endReadSlice();
	}
    }
    
    public void
    ice_addFacet(Object facet, string name)
    {
	lock(_activeFacetMap)
	{
	    Object o = (Object)_activeFacetMap[name];
	    if(o != null)
	    {
		Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
		ex.id = name;
		ex.kindOfObject = "facet";
		throw ex;
	    }
	    _activeFacetMap[name] = facet;
	}
    }
    
    public Object
    ice_removeFacet(string name)
    {
	lock(_activeFacetMap)
	{
	    Object o = (Object)_activeFacetMap[name];
	    if(o == null)
	    {
		Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
		ex.id = name;
		ex.kindOfObject = "facet";
		throw ex;
	    }
	    _activeFacetMap.Remove(name);
	    return o;
	}
    }
    
    public void
    ice_removeAllFacets()
    {
	lock(_activeFacetMap)
	{
	    _activeFacetMap.Clear();
	}
    }
    
    public Object
    ice_findFacet(string name)
    {
	lock(_activeFacetMap)
	{
	    return (Object)_activeFacetMap[name];
	}
    }
    
    public Object
    ice_findFacetPath(FacetPath path, int start)
    {
	int sz = path.Count;
	
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
    
    private Hashtable _activeFacetMap;
}

}
