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

package IceInternal;

public final class ObjectFactoryManager
{
    public synchronized void
    add(Ice.ObjectFactory factory, String id)
    {
        Object o = _factoryMap.get(id);
	if(o != null)
	{
	    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
	    ex.id = id;
	    ex.kindOfObject = "object factory";
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
	    ex.kindOfObject = "object factory";
	    throw ex;
	}

        ((Ice.ObjectFactory)o).destroy();

        _factoryMap.remove(id);
    }

    public synchronized Ice.ObjectFactory
    find(String id)
    {
        return (Ice.ObjectFactory)_factoryMap.get(id);
    }

    //
    // Only for use by Instance
    //
    ObjectFactoryManager()
    {
    }

    synchronized void
    destroy()
    {
        java.util.Iterator i = _factoryMap.values().iterator();
        while(i.hasNext())
        {
            Ice.ObjectFactory factory = (Ice.ObjectFactory)i.next();
            factory.destroy();
        }
        _factoryMap.clear();
    }

    private java.util.HashMap _factoryMap = new java.util.HashMap();
}
