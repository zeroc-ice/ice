// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    using System.Collections;

    public sealed class ObjectFactoryManager
    {
	public void add(Ice.ObjectFactory factory, string id)
	{
	    lock(this)
	    {
		object o = _factoryMap[id];
		if(o != null)
		{
		    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
		    ex.id = id;
		    ex.kindOfObject = "user exception factory";
		    throw ex;
		}
		_factoryMap[id] = factory;
	    }
	}
	
	public void remove(string id)
	{
	    lock(this)
	    {
		object o = _factoryMap[id];
		if(o == null)
		{
		    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
		    ex.id = id;
		    ex.kindOfObject = "user exception factory";
		    throw ex;
		}
		_factoryMap.Remove(id);
	    }
	}
	
	public Ice.ObjectFactory find(string id)
	{
	    lock(this)
	    {
		return (Ice.ObjectFactory)_factoryMap[id];
	    }
	}
	
	//
	// Only for use by Instance
	//
	internal ObjectFactoryManager()
	{
	    _factoryMap = new Hashtable();
	}
	
	internal void destroy()
	{
	    foreach(Ice.ObjectFactory factory in _factoryMap.Values)
	    {
		factory.destroy();
	    }
	    _factoryMap.Clear();
	}
	
	private Hashtable _factoryMap;
    }

}
