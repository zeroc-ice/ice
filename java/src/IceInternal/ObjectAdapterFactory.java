// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class ObjectAdapterFactory
{
    public synchronized void
    shutdown()
    {
        java.util.Iterator i = _adapters.values().iterator();
        while (i.hasNext())
        {
            Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.next();
            adapter.deactivate();
        }
        _adapters.clear();
    }

    public synchronized Ice.ObjectAdapter
    createObjectAdapter(String name, String endpts)
    {
        Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)_adapters.get(name);
        if (adapter != null)
        {
            return adapter;
        }

        adapter = new Ice.ObjectAdapterI(_instance, name, endpts);
        _adapters.put(name, adapter);
        return adapter;
    }

    public Ice.ObjectAdapter
    findObjectAdapter(Ice.ObjectPrx proxy)
    {
        java.util.Iterator i = _adapters.values().iterator();
        while (i.hasNext())
        {
            Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)i.next();
            if (adapter.isLocal(proxy))
            {
                return adapter;
            }
        }

        return null;
    }

    //
    // Only for use by Instance
    //
    ObjectAdapterFactory(Instance instance)
    {
        _instance = instance;
    }

    private Instance _instance;
    private java.util.HashMap _adapters = new java.util.HashMap();
}
