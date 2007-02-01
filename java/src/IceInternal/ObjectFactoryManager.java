// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    public void
    remove(String id)
    {
        Object o = null;

        synchronized(this)
        {
            o = _factoryMap.get(id);
            if(o == null)
            {
                Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                ex.id = id;
                ex.kindOfObject = "object factory";
                throw ex;
            }
            _factoryMap.remove(id);
        }

        ((Ice.ObjectFactory)o).destroy();   
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

    void
    destroy()
    {
        java.util.Map oldMap = null;
        synchronized(this)
        {
            oldMap = _factoryMap;
            _factoryMap = new java.util.HashMap();
        }

        java.util.Iterator i = oldMap.values().iterator();
        while(i.hasNext())
        {
            Ice.ObjectFactory factory = (Ice.ObjectFactory)i.next();
            factory.destroy();
        }
    }

    private java.util.HashMap _factoryMap = new java.util.HashMap();
}
