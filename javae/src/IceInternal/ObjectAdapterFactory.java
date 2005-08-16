// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

        java.util.Enumeration i = _adapters.elements();
        while(i.hasMoreElements())
        {
            Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.nextElement();
            adapter.deactivate();
        }

	_instance = null;
	_communicator = null;
	
	notifyAll();
    }

    public void
    waitForShutdown()
    {
	synchronized(this)
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
	    // If some other thread is currently shutting down, we wait
	    // until this thread is finished.
	    //
	    while(_waitForShutdown)
	    {
		try
		{
		    wait();
		}
		catch(InterruptedException ex)
		{
		}
	    }
	    _waitForShutdown = true;
	}
	
	//
	// Now we wait for deactivation of each object adapter.
	//
	if(_adapters != null)
	{
	    java.util.Enumeration i = _adapters.elements();
	    while(i.hasMoreElements())
	    {
		Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.nextElement();
		adapter.waitForDeactivate();
	    }
	}

	//
	// We're done, now we can throw away the object adapters.
	//
	_adapters = null;

	synchronized(this)
	{
	    //
	    // Signal that waiting is complete.
	    //
	    _waitForShutdown = false;
	    notifyAll();
	}
    }
    
    public synchronized Ice.ObjectAdapter
    createObjectAdapter(String name)
    {
	if(_instance == null)
	{
	    throw new Ice.ObjectAdapterDeactivatedException();
	}

        Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)_adapters.get(name);
        if(adapter != null)
        {
            return adapter;
        }

        adapter = new Ice.ObjectAdapter(_instance, _communicator, name);
        _adapters.put(name, adapter);
        return adapter;
    }

    public synchronized Ice.ObjectAdapter
    findObjectAdapter(Ice.ObjectPrx proxy)
    {
	if(_instance == null)
	{
	    return null;
	}

        java.util.Enumeration i = _adapters.elements();
        while(i.hasMoreElements())
        {
            Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.nextElement();
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

    public void
    flushBatchRequests()
    {
	java.util.Vector a = new java.util.Vector();
        synchronized(this)
	{
	    java.util.Enumeration i = _adapters.elements();
	    while(i.hasMoreElements())
	    {
	        a.addElement(i.nextElement());
	    }
	}
	java.util.Enumeration p = a.elements();
	while(p.hasMoreElements())
	{
	    ((Ice.ObjectAdapter)p.nextElement()).flushBatchRequests();
	}
    }

    //
    // Only for use by Instance.
    //
    ObjectAdapterFactory(Instance instance, Ice.Communicator communicator)
    {
        _instance = instance;
	_communicator = communicator;
	_waitForShutdown = false;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
	IceUtil.Debug.FinalizerAssert(_instance == null);
	IceUtil.Debug.FinalizerAssert(_communicator == null);
	IceUtil.Debug.FinalizerAssert(_adapters == null);
	IceUtil.Debug.FinalizerAssert(!_waitForShutdown);

        // Cannot call finalize on superclass. java.lang.Object.finalize() not available in CLDC.
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
    private java.util.Hashtable _adapters = new java.util.Hashtable();
    private boolean _waitForShutdown;
}
