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


package IceInternal;

public final class UserExceptionFactoryManager
{
    public synchronized void
    add(UserExceptionFactory factory, String id)
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

    public synchronized UserExceptionFactory
    find(String id)
    {
        return (UserExceptionFactory)_factoryMap.get(id);
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
            UserExceptionFactory factory = (UserExceptionFactory)i.next();
            factory.destroy();
        }
        _factoryMap.clear();
    }

    private java.util.HashMap _factoryMap = new java.util.HashMap();
}
