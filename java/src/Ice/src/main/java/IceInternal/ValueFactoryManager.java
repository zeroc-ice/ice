// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

@SuppressWarnings("deprecation")
public final class ValueFactoryManager
{
    public synchronized void
    add(Ice.ObjectFactory factory, String id)
    {
        Object o = _factoryMap.get(id);
        if(o != null)
        {
            Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
            ex.id = id;
            ex.kindOfObject = "value factory";
            throw ex;
        }

        _objectFactoryMap.put(id, factory);
        _factoryMap.put(id, new ObjectFactoryWrapper(factory));
    }

    public synchronized void
    add(Ice.ValueFactory factory, String id)
    {
        Object o = _factoryMap.get(id);
        if(o != null)
        {
            Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
            ex.id = id;
            ex.kindOfObject = "value factory";
            throw ex;
        }
        _factoryMap.put(id, factory);
    }

    public synchronized Ice.ValueFactory
    find(String id)
    {
        return _factoryMap.get(id);
    }

    public synchronized Ice.ObjectFactory
    findObjectFactory(String id)
    {
        return _objectFactoryMap.get(id);
    }

    //
    // Only for use by Instance
    //
    ValueFactoryManager()
    {
    }

    void
    destroy()
    {
        java.util.Map<String, Ice.ObjectFactory> oldMap = null;
        synchronized(this)
        {
            _factoryMap = new java.util.HashMap<String, Ice.ValueFactory>();


            oldMap = _objectFactoryMap;
            _objectFactoryMap = new java.util.HashMap<String, Ice.ObjectFactory>();

        }

        for(Ice.ObjectFactory factory : oldMap.values())
        {
            factory.destroy();
        }
    }

    private class ObjectFactoryWrapper implements Ice.ValueFactory
    {
        public ObjectFactoryWrapper(Ice.ObjectFactory factory)
        {
            _objectFactory = factory;
        }

        public Ice.Object create(String id)
        {
            return _objectFactory.create(id);
        }

        private final Ice.ObjectFactory _objectFactory;
    }

    private java.util.Map<String, Ice.ValueFactory> _factoryMap = new java.util.HashMap<String, Ice.ValueFactory>();
    private java.util.Map<String, Ice.ObjectFactory> _objectFactoryMap = new java.util.HashMap<String, Ice.ObjectFactory>();
}
