// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        java.util.HashMap adapters;
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

            _instance = null;
            _communicator = null;

            adapters = _adapters;
        
            notifyAll();
        }

        //
        // Deactivate outside the thread synchronization, to avoid
        // deadlocks.
        //
        if(adapters != null)
        {
            java.util.Iterator i = adapters.values().iterator();
            while(i.hasNext())
            {
                Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.next();
                adapter.deactivate();
            }
        }
    }

    public void
    waitForShutdown()
    {
        java.util.HashMap adapters;
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
            adapters = _adapters;
        }
        
        //
        // Now we wait for deactivation of each object adapter.
        //
        if(adapters != null)
        {
            java.util.Iterator i = adapters.values().iterator();
            while(i.hasNext())
            {
                Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.next();
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

        java.util.HashMap adapters;
        synchronized(this)
        {
            adapters = _adapters;

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
        if(adapters != null)
        {
            java.util.Iterator i = adapters.values().iterator();
            while(i.hasNext())
            {
                Ice.ObjectAdapter adapter = (Ice.ObjectAdapter)i.next();
                adapter.destroy();
            }
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

        if(name.length() == 0 && (endpoints.length() != 0 || router != null))
        {
            Ice.InitializationException ex = new Ice.InitializationException();
            ex.reason = "Cannot configure endpoints or router with nameless object adapter";
            throw ex;
        }

        if(name.length() == 0)
        {
            String uuid = Ice.Util.generateUUID();
            adapter = new Ice.ObjectAdapterI(_instance, _communicator, this, uuid, "", null, true);
            _adapters.put(uuid, adapter);
        }
        else
        {
            adapter = new Ice.ObjectAdapterI(_instance, _communicator, this, name, endpoints, router, false);
            _adapters.put(name, adapter);
        }
        return adapter;
    }

    public Ice.ObjectAdapter
    findObjectAdapter(Ice.ObjectPrx proxy)
    {
        java.util.ArrayList adapters;
        synchronized(this)
        {
            if(_instance == null)
            {
                return null;
            }

            adapters = new java.util.ArrayList(_adapters.values());
        }

        java.util.Iterator p = adapters.iterator();
        while(p.hasNext())
        {
            try
            {
                Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)p.next();
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
    removeObjectAdapter(String name)
    {
        if(_instance == null)
        {
            return;
        }

        _adapters.remove(name);
    }

    public void
    flushBatchRequests()
    {
        java.util.ArrayList adapters;
        synchronized(this)
        {
            if(_adapters == null)
            {
                return;
            }

            adapters = new java.util.ArrayList(_adapters.values());
        }

        java.util.Iterator p = adapters.iterator();
        while(p.hasNext())
        {
            ((Ice.ObjectAdapterI)p.next()).flushBatchRequests();
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
        IceUtil.Assert.FinalizerAssert(_instance == null);
        IceUtil.Assert.FinalizerAssert(_communicator == null);
        IceUtil.Assert.FinalizerAssert(_adapters == null);
        IceUtil.Assert.FinalizerAssert(!_waitForShutdown);

        super.finalize();
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
    private java.util.HashMap _adapters = new java.util.HashMap();
    private boolean _waitForShutdown;
}
