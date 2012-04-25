// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class EndpointHostResolver
{
    EndpointHostResolver(Instance instance)
    {
        _instance = instance;
        try
        {
            _thread = new HelperThread();
            if(_instance.initializationData().properties.getProperty("Ice.ThreadPriority").length() > 0)
            {
                _thread.setPriority(Util.getThreadPriorityProperty(_instance.initializationData().properties, "Ice"));
            }
            _thread.start();
        }
        catch(RuntimeException ex)
        {
            String s = "cannot create thread for endpoint host resolver thread:\n" + Ex.toString(ex);
            _instance.initializationData().logger.error(s);
            throw ex;
        }
    }

    synchronized public void
    resolve(String host, int port, EndpointI endpoint, EndpointI_connectors callback)
    {
        //
        // TODO: Optimize to avoid the lookup if the given host is a textual IPv4 or IPv6
        // address. This requires implementing parsing of IPv4/IPv6 addresses (Java does
        // not provide such methods).
        //

        assert(!_destroyed);

        ResolveEntry entry = new ResolveEntry();
        entry.host = host;
        entry.port = port;
        entry.endpoint = endpoint;
        entry.callback = callback;
        _queue.add(entry);
        notify();
    }

    synchronized public void
    destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        notify();
    }

    public void
    joinWithThread()
    {
        if(_thread != null)
        {
            try
            {
                _thread.join();
            }
            catch(InterruptedException ex)
            {
            }
        }
    }

    public void
    run()
    {
        while(true)
        {
            ResolveEntry resolve;
            synchronized(this)
            {
                while(!_destroyed && _queue.isEmpty())
                {
                    try
                    {
                        wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }

                if(_destroyed)
                {
                    break;
                }

                resolve = (ResolveEntry)_queue.removeFirst();
            }

            try
            {
                resolve.callback.connectors(
                    resolve.endpoint.connectors(
                        Network.getAddresses(resolve.host, resolve.port, _instance.protocolSupport())));
            }
            catch(Ice.LocalException ex)
            {
                resolve.callback.exception(ex);
            }
        }

        for(ResolveEntry p : _queue)
        {
            p.callback.exception(new Ice.CommunicatorDestroyedException());
        }
        _queue.clear();
    }

    static class ResolveEntry
    {
        String host;
        int port;
        EndpointI endpoint;
        EndpointI_connectors callback;
    }

    private final Instance _instance;
    private boolean _destroyed;
    private java.util.LinkedList<ResolveEntry> _queue = new java.util.LinkedList<ResolveEntry>();

    private final class HelperThread extends Thread
    {
        HelperThread()
        {
            String threadName = _instance.initializationData().properties.getProperty("Ice.ProgramName");
            if(threadName.length() > 0)
            {
                threadName += "-";
            }
            setName(threadName + "Ice.EndpointHostResolverThread");
        }

        public void
        run()
        {
            try
            {
                EndpointHostResolver.this.run();
            }
            catch(java.lang.Exception ex)
            {
                String s = "exception in endpoint host resolver thread " + getName() + ":\n" + Ex.toString(ex);
                _instance.initializationData().logger.error(s);
            }
        }
    }

    private HelperThread _thread;
}
