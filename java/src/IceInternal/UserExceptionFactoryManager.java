// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class UserExceptionFactoryManager
{
    public synchronized void
    add(Ice.UserExceptionFactory factory, String id)
    {
        _factoryMap.put(id, factory);
    }

    public synchronized void
    remove(String id)
    {
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
