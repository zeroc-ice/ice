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

namespace IceInternal
{

    using System.Collections;

    public sealed class UserExceptionFactoryManager
    {
	public void add(UserExceptionFactory factory, string id)
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
	
	public UserExceptionFactory find(string id)
	{
	    lock(this)
	    {
		return (UserExceptionFactory)_factoryMap[id];
	    }
	}
	
	//
	// Only for use by Instance
	//
	internal UserExceptionFactoryManager()
	{
	    _factoryMap = new Hashtable();
	}
	
	internal void destroy()
	{
	    foreach(UserExceptionFactory factory in _factoryMap.Values)
	    {
		factory.destroy();
	    }
	    _factoryMap.Clear();
	}
	
	private Hashtable _factoryMap;
    }

}
