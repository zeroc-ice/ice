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

public final class ObjectAdapterFactory
{
    public synchronized void
    shutdown()
    {
	//
	// Ignore shutdown requests if the object adapter factory has
	// already been shut down.
	//
	if(_instance == null)
	{
	    return;
	}

        java.util.Iterator i = _adapters.values().iterator();
        while(i.hasNext())
        {
            Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.next();
            adapter.deactivate();
        }

	_instance = null;
	_communicator = null;
	
	notifyAll();
    }

    public synchronized void
    waitForShutdown()
    {
	//
	// First we wait for the shutdown of the factory itself.
	//
	while(_instance != null)
	{
	    try
	    {
		wait();
	    }
	    catch(InterruptedException ex)
	    {
	    }
	}
	
	//
	// Now we wait for deactivation of each object adapter.
	//
        java.util.Iterator i = _adapters.values().iterator();
        while(i.hasNext())
        {
            Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.next();
            adapter.waitForDeactivate();
        }
	
	//
	// We're done, now we can throw away the object adapters.
	//
	_adapters.clear();
    }
    
    public synchronized Ice.ObjectAdapter
    createObjectAdapter(String name, String endpts, String id)
    {
	if(_instance == null)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

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
	if(_instance == null)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

        java.util.Iterator i = _adapters.values().iterator();
        while(i.hasNext())
        {
            Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)i.next();
	    try
	    {
		if(adapter.isLocal(proxy))
		{
		    return adapter;
		}
	    }
	    catch(Ice.ObjectAdapterDeactivatedException ex)
	    {
		// Ignore.
	    }
	}

        return null;
    }

    //
    // Only for use by Instance.
    //
    ObjectAdapterFactory(Instance instance, Ice.Communicator communicator)
    {
        _instance = instance;
	_communicator = communicator;
    }

    protected void
    finalize()
        throws Throwable
    {
	assert(_instance == null);
	assert(_communicator == null);
	assert(_adapters.size() == 0);

        super.finalize();
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
    private java.util.HashMap _adapters = new java.util.HashMap();
}
