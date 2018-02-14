// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ObjectAdapterFactory
{
    public void
    shutdown()
    {
        java.util.List<Ice.ObjectAdapterI> adapters;
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

            adapters = new java.util.LinkedList<Ice.ObjectAdapterI>(_adapters);
        }

        //
        // Deactivate outside the thread synchronization, to avoid
        // deadlocks.
        //
        for(Ice.ObjectAdapterI adapter : adapters)
        {
            adapter.deactivate();
        }

        synchronized(this)
        {
            _instance = null;
            _communicator = null;
            notifyAll();
        }
    }

    public void
    waitForShutdown()
    {
        java.util.List<Ice.ObjectAdapterI> adapters;
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
                    throw new Ice.OperationInterruptedException();
                }
            }

            adapters = new java.util.LinkedList<Ice.ObjectAdapterI>(_adapters);
        }

        //
        // Now we wait for deactivation of each object adapter.
        //
        for(Ice.ObjectAdapterI adapter : adapters)
        {
            adapter.waitForDeactivate();
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

        java.util.List<Ice.ObjectAdapterI> adapters;
        synchronized(this)
        {
            adapters = new java.util.LinkedList<Ice.ObjectAdapterI>(_adapters);
        }

        for(Ice.ObjectAdapterI adapter : adapters)
        {
            adapter.destroy();
        }

        synchronized(this)
        {
            _adapters.clear();
        }
    }

    public void
    updateConnectionObservers()
    {
        java.util.List<Ice.ObjectAdapterI> adapters;
        synchronized(this)
        {
            adapters = new java.util.LinkedList<Ice.ObjectAdapterI>(_adapters);
        }

        for(Ice.ObjectAdapterI adapter : adapters)
        {
            adapter.updateConnectionObservers();
        }
    }

    public void
    updateThreadObservers()
    {
        java.util.List<Ice.ObjectAdapterI> adapters;
        synchronized(this)
        {
            adapters = new java.util.LinkedList<Ice.ObjectAdapterI>(_adapters);
        }

        for(Ice.ObjectAdapterI adapter : adapters)
        {
            adapter.updateThreadObservers();
        }
    }

    public synchronized Ice.ObjectAdapter
    createObjectAdapter(String name, Ice.RouterPrx router)
    {
        if(Thread.interrupted())
        {
            throw new Ice.OperationInterruptedException();
        }

        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Ice.ObjectAdapterI adapter = null;
        if(name.length() == 0)
        {
            String uuid = java.util.UUID.randomUUID().toString();
            adapter = new Ice.ObjectAdapterI(_instance, _communicator, this, uuid, null, true);
        }
        else
        {
            if(_adapterNamesInUse.contains(name))
            {
                throw new Ice.AlreadyRegisteredException("object adapter", name);
            }
            adapter = new Ice.ObjectAdapterI(_instance, _communicator, this, name, router, false);
            _adapterNamesInUse.add(name);
        }
        _adapters.add(adapter);
        return adapter;
    }

    public Ice.ObjectAdapter
    findObjectAdapter(Ice.ObjectPrx proxy)
    {
        java.util.List<Ice.ObjectAdapterI> adapters;
        synchronized(this)
        {
            if(_instance == null)
            {
                return null;
            }

            adapters = new java.util.LinkedList<Ice.ObjectAdapterI>(_adapters);
        }

        for(Ice.ObjectAdapterI adapter : adapters)
        {
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

    public synchronized void
    removeObjectAdapter(Ice.ObjectAdapter adapter)
    {
        if(_instance == null)
        {
            return;
        }

        _adapters.remove(adapter);
        _adapterNamesInUse.remove(adapter.getName());
    }

    public void
    flushAsyncBatchRequests(Ice.CompressBatch compressBatch, CommunicatorFlushBatch outAsync)
    {
        java.util.List<Ice.ObjectAdapterI> adapters;
        synchronized(this)
        {
            adapters = new java.util.LinkedList<Ice.ObjectAdapterI>(_adapters);
        }

        for(Ice.ObjectAdapterI adapter : adapters)
        {
            adapter.flushAsyncBatchRequests(compressBatch, outAsync);
        }
    }

    //
    // Only for use by Instance.
    //
    ObjectAdapterFactory(Instance instance, Ice.Communicator communicator)
    {
        _instance = instance;
        _communicator = communicator;
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_instance == null);
            IceUtilInternal.Assert.FinalizerAssert(_communicator == null);
            IceUtilInternal.Assert.FinalizerAssert(_adapters.isEmpty());
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
    private java.util.Set<String> _adapterNamesInUse = new java.util.HashSet<String>();
    private java.util.List<Ice.ObjectAdapterI> _adapters = new java.util.LinkedList<Ice.ObjectAdapterI>();
}
