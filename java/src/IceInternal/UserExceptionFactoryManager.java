// **********************************************************************
//
// Copyright (c) 2002
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

package IceInternal;

public final class UserExceptionFactoryManager
{
    public synchronized void
    add(Ice.UserExceptionFactory factory, String id)
    {
        Object o = _factoryMap.get(id);
	if(o != null)
	{
	    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
	    ex.id = id;
	    ex.kindOfObject = "user exception factory";
	    throw ex;
	}
        _factoryMap.put(id, factory);
    }

    public synchronized void
    remove(String id)
    {
        Object o = _factoryMap.get(id);
	if(o == null)
	{
	    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
	    ex.id = id;
	    ex.kindOfObject = "user exception factory";
	    throw ex;
	}
        _factoryMap.remove(id);
    }

    public synchronized Ice.UserExceptionFactory
    find(String id)
    {
        return (Ice.UserExceptionFactory)_factoryMap.get(id);
    }

    //
    // Only for use by Instance
    //
    UserExceptionFactoryManager()
    {
    }

    void
    destroy()
    {
        java.util.Iterator i = _factoryMap.values().iterator();
        while(i.hasNext())
        {
            Ice.UserExceptionFactory factory =
                (Ice.UserExceptionFactory)i.next();
            factory.destroy();
        }
        _factoryMap.clear();
    }

    private java.util.HashMap _factoryMap = new java.util.HashMap();
}
