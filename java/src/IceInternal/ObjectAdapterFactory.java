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

public final class ObjectAdapterFactory
{
    public synchronized void
    shutdown()
    {
        java.util.Iterator i = _adapters.values().iterator();
        while(i.hasNext())
        {
            Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.next();
            adapter.deactivate();
        }

        _adapters.clear();
    }

    public synchronized Ice.ObjectAdapter
    createObjectAdapter(String name, String endpts, String id)
    {
        Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)_adapters.get(name);
        if(adapter != null)
        {
            return adapter;
        }

        adapter = new Ice.ObjectAdapterI(_instance, _communicator, name, endpts, id);
        _adapters.put(name, adapter);
        return adapter;
    }

    public synchronized Ice.ObjectAdapter
    findObjectAdapter(Ice.ObjectPrx proxy)
    {
        java.util.Iterator i = _adapters.values().iterator();
        while(i.hasNext())
        {
            Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)i.next();
            if(adapter.isLocal(proxy))
            {
                return adapter;
            }
        }

        return null;
    }

    //
    // Only for use by Instance
    //
    ObjectAdapterFactory(Instance instance, Ice.Communicator communicator)
    {
        _instance = instance;
	_communicator = communicator;
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
    private java.util.HashMap _adapters = new java.util.HashMap();
}
