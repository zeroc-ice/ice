// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ObjectAdapterFactory
{
    public void
    shutdown()
    {
        Ice.ObjectAdapter[] adapters = null;

        synchronized(this)
	{
	    //
	    // Ignore shutdown requests if the object adapter factory has
	    // already been shut down.
	    //
	    if(_instance == null)
	    {
	        return;
	    }

	    adapters = new Ice.ObjectAdapter[_adapters.size()];
	    int i = 0;
	    java.util.Enumeration e = _adapters.elements();
	    while(e.hasMoreElements())
	    {
	        adapters[i++] = (Ice.ObjectAdapter)e.nextElement();
	    }

	    _instance = null;
	    _communicator = null;
	
	    notifyAll();
	}

	//
	// Deactivate outside the thread synchronization, to avoid
	// deadlocks.
	//
	for(int i = 0; i < adapters.length; ++i)
	{
	    adapters[i].deactivate();
	}
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

	synchronized(this)
	{
	    //
	    // Signal that waiting is complete.
	    //
	    _waitForShutdown = false;
	    notifyAll();
	}
    }

    public synchronized boolean
    isShutdown()
    {
        return _instance == null;
    }

    public void
    destroy()
    {
        //
        // First wait for shutdown to finish.
        //
        waitForShutdown();

        Ice.ObjectAdapter[] adapters;

        synchronized(this)
        {
            adapters = new Ice.ObjectAdapter[_adapters.size()];
            int idx = 0;
            for(java.util.Enumeration e = _adapters.elements(); e.hasMoreElements(); )
            {
                adapters[idx] = (Ice.ObjectAdapter)e.nextElement();
                idx++;
            }

            //
            // For consistency with C#, we set _adapters to null
            // so that our finalizer does not try to invoke any
            // methods on member objects.
            //
            _adapters = null;
        }

        //
        // Now we destroy each object adapter.
        //
        for(int i = 0; i < adapters.length; ++i)
        {
            adapters[i].destroy();
        }
    }
    
    public synchronized Ice.ObjectAdapter
    createObjectAdapter(String name, String endpoints, Ice.RouterPrx router)
    {
	if(_instance == null)
	{
	    throw new Ice.ObjectAdapterDeactivatedException();
	}

        Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)_adapters.get(name);
        if(adapter != null)
        {
	    throw new Ice.AlreadyRegisteredException("object adapter", name);
        }

        adapter = new Ice.ObjectAdapter(_instance, _communicator, this, name, endpoints, router);
        _adapters.put(name, adapter);
        return adapter;
    }

    synchronized public void
    removeObjectAdapter(String name)
    {
        if(_waitForShutdown || _adapters == null)
	{
	    return;
	}

	_adapters.remove(name);
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
