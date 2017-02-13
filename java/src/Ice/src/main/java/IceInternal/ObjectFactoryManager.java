// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
        Ice.ObjectFactory factory = null;

        synchronized(this)
        {
            factory = _factoryMap.get(id);
            if(factory == null)
            {
                Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                ex.id = id;
                ex.kindOfObject = "object factory";
                throw ex;
            }
            _factoryMap.remove(id);
        }

        factory.destroy();
    }

    public synchronized Ice.ObjectFactory
    find(String id)
    {
        return _factoryMap.get(id);
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
        java.util.Map<String, Ice.ObjectFactory> oldMap = null;
        synchronized(this)
        {
            oldMap = _factoryMap;
            _factoryMap = new java.util.HashMap<String, Ice.ObjectFactory>();
        }

        for(Ice.ObjectFactory factory : oldMap.values())
        {
            factory.destroy();
        }
    }

    private java.util.Map<String, Ice.ObjectFactory> _factoryMap = new java.util.HashMap<String, Ice.ObjectFactory>();
}
