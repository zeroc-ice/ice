// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectAdapterI;

public final class ObjectAdapterFactory
{
    public void
    shutdown()
    {
        java.util.List<ObjectAdapterI> adapters;
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

            adapters = new java.util.LinkedList<>(_adapters);
        }

        //
        // Deactivate outside the thread synchronization, to avoid
        // deadlocks.
        //
        for(ObjectAdapterI adapter : adapters)
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
        java.util.List<ObjectAdapterI> adapters;
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
                    throw new com.zeroc.Ice.OperationInterruptedException();
                }
            }

            adapters = new java.util.LinkedList<>(_adapters);
        }

        //
        // Now we wait for deactivation of each object adapter.
        //
        for(ObjectAdapterI adapter : adapters)
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

        java.util.List<ObjectAdapterI> adapters;
        synchronized(this)
        {
            adapters = new java.util.LinkedList<>(_adapters);
        }

        for(ObjectAdapterI adapter : adapters)
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
        java.util.List<ObjectAdapterI> adapters;
        synchronized(this)
        {
            adapters = new java.util.LinkedList<>(_adapters);
        }

        for(ObjectAdapterI adapter : adapters)
        {
            adapter.updateConnectionObservers();
        }
    }

    public void
    updateThreadObservers()
    {
        java.util.List<ObjectAdapterI> adapters;
        synchronized(this)
        {
            adapters = new java.util.LinkedList<>(_adapters);
        }

        for(ObjectAdapterI adapter : adapters)
        {
            adapter.updateThreadObservers();
        }
    }

    public synchronized ObjectAdapter
    createObjectAdapter(String name, com.zeroc.Ice.RouterPrx router)
    {
        if(Thread.interrupted())
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }

        if(_instance == null)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        ObjectAdapterI adapter = null;
        if(name.length() == 0)
        {
            String uuid = java.util.UUID.randomUUID().toString();
            adapter = new ObjectAdapterI(_instance, _communicator, this, uuid, null, true);
        }
        else
        {
            if(_adapterNamesInUse.contains(name))
            {
                throw new com.zeroc.Ice.AlreadyRegisteredException("object adapter", name);
            }
            adapter = new ObjectAdapterI(_instance, _communicator, this, name, router, false);
            _adapterNamesInUse.add(name);
        }
        _adapters.add(adapter);
        return adapter;
    }

    public ObjectAdapter
    findObjectAdapter(com.zeroc.Ice.ObjectPrx proxy)
    {
        java.util.List<ObjectAdapterI> adapters;
        synchronized(this)
        {
            if(_instance == null)
            {
                return null;
            }

            adapters = new java.util.LinkedList<>(_adapters);
        }

        for(ObjectAdapterI adapter : adapters)
        {
            try
            {
                if(adapter.isLocal(proxy))
                {
                    return adapter;
                }
            }
            catch(com.zeroc.Ice.ObjectAdapterDeactivatedException ex)
            {
                // Ignore.
            }
        }

        return null;
    }

    public synchronized void
    removeObjectAdapter(ObjectAdapter adapter)
    {
        if(_instance == null)
        {
            return;
        }

        _adapters.remove(adapter);
        _adapterNamesInUse.remove(adapter.getName());
    }

    public void
    flushAsyncBatchRequests(com.zeroc.Ice.CompressBatch compressBatch, CommunicatorFlushBatch outAsync)
    {
        java.util.List<ObjectAdapterI> adapters;
        synchronized(this)
        {
            adapters = new java.util.LinkedList<>(_adapters);
        }

        for(ObjectAdapterI adapter : adapters)
        {
            adapter.flushAsyncBatchRequests(compressBatch, outAsync);
        }
    }

    //
    // Only for use by Instance.
    //
    ObjectAdapterFactory(Instance instance, com.zeroc.Ice.Communicator communicator)
    {
        _instance = instance;
        _communicator = communicator;
    }

    @Override
    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_instance == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_communicator == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_adapters.isEmpty());
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
    private com.zeroc.Ice.Communicator _communicator;
    private java.util.Set<String> _adapterNamesInUse = new java.util.HashSet<>();
    private java.util.List<ObjectAdapterI> _adapters = new java.util.LinkedList<>();
}
