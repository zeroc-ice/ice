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

public class CollectorFactory extends EventHandler
{
    public synchronized void
    destroy()
    {
        setState(StateClosed);
    }

    public synchronized void
    hold()
    {
        setState(StateHolding);
    }

    public synchronized void
    activate()
    {
        setState(StateActive);
    }

    public Endpoint
    endpoint()
    {
        return _endpoint;
    }

    public boolean
    equivalent(Endpoint endp)
    {
        if (_transceiver != null)
        {
            return endp.equivalent(_transceiver);
        }

        assert(_acceptor != null);
        return endp.equivalent(_acceptor);
    }

    //
    // Operations from EventHandler
    //
    public boolean
    server()
    {
        return true;
    }

    public boolean
    readable()
    {
        return false;
    }

    public void
    read(BasicStream is)
    {
        assert(false); // Must not be called
    }

    public synchronized void
    message(BasicStream stream)
    {
        _threadPool.promoteFollower();

        if (_state != StateActive)
        {
            return;
        }

        //
        // First reap destroyed collectors
        //
        java.util.ListIterator i = _collectors.listIterator();
        while (i.hasNext())
        {
            Collector c = (Collector)i.next();
            if (c.destroyed())
            {
                i.remove();
            }
        }

        //
        // Now accept a new connection and create a new Collector
        //
        try
        {
            Transceive transceiver = _acceptor.accept(0);
            Collector collector = new Collector(_instance, _adapter,
                                                transceiver, _endpoint);
            collector.activate();
            _collectors.add(collector);
        }
        catch (SecurityException ex)
        {
            // Ignore, nothing we can do here
        }
        catch (SocketException ex)
        {
            // Ignore, nothing we can do here
        }
        catch (TimeoutException ex)
        {
            // Ignore timeouts
        }
        catch (LocalException ex)
        {
            warning(ex);
            setState(StateClosed);
        }
    }

    public void
    exception(Ice.LocalException ex)
    {
        assert(false); // Must not be called
    }

    public synchronized void
    finished()
    {
        //
        // We also unregister with the thread pool if we go to holding
        // state, but in this case we may not close the connection.
        //
        if (_state == StateClosed)
        {
            _acceptor.shutdown();
            clearBacklog();
            _acceptor.close();
        }
    }

    public boolean
    tryDestroy()
    {
        //
        // Do nothing. We don't want collector factories to be closed by
        // active connection management.
        //
        return false;
    }

    //
    // Only for use by Ice.ObjectAdapterI
    //
    public
    CollectorFactory(Instance instance,
                     Ice.ObjectAdapter adapter,
                     Endpoint endpoint)
    {
        super(instance);
        _adapter = adapter;
        _endpoint = endpoint;
        _traceLevels = instance.traceLevels();
        _logger = instance.logger();
        _state = StateHolding;

        try
        {
            String value = instance.properties().getProperty(
                "Ice.WarnAboutServerExceptions");
            _warnAboutExceptions = Integer.parseInt(value) > 0 ? true : false;
        }
        catch (NumberFormatException ex)
        {
            _warnAboutExceptions = false;
        }

        try
        {
            _transceiver = _endpoint.serverTransceiver(_instance, _endpoint);
            if (_transceiver != null)
            {
                Collector collector = new Collector(_instance, _adapter,
                                                    _transceiver, _endpoint);
                _collectors.add(collector);
            }
            else
            {
                _acceptor = _endpoint.acceptor(_instance, _endpoint);
                assert(_acceptor != null);
                _acceptor.listen();
                _threadPool = instance.threadPool();
            }
        }
        catch (Ice.LocalException ex)
        {
            setState(StateClosed);
            throw ex;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_state == StateClosed);

        super.finalize();
    }

    private static final int StateActive = 0;
    private static final int StateHolding = 1;
    private static final int StateClosing = 2;
    private static final int StateClosed = 3;

    private void
    setState(int state)
    {
        if (_state == state) // Don't switch twice
        {
            return;
        }

        switch (state)
        {
            case StateActive:
            {
                if (_state != StateHolding) // Can only switch from holding
                {                           // to active
                    return;
                }

                if (_threadPool != null)
                {
                    _threadPool._register(_acceptor.fd(), this);
                }

                java.util.ListIterator i = _collectors.listIterator();
                while (i.hasNext())
                {
                    Collector c = (Collector)i.next();
                    c.activate();
                }

                break;
            }

            case StateHolding:
            {
                if (_state != StateActive) // Can only switch from active
                {                           // to holding
                    return;
                }

                if (_threadPool != null)
                {
                    _threadPool.unregister(_acceptor.fd());
                }

                java.util.ListIterator i = _collectors.listIterator();
                while (i.hasNext())
                {
                    Collector c = (Collector)i.next();
                    c.hold();
                }

                break;
            }

            case StateClosed:
            {
                if (_threadPool != null)
                {
                    //
                    // If we come from holding state, we are already
                    // unregistered, so we can close right away.
                    //
                    if (_state == StateHolding)
                    {
                        _acceptor.shutdown();
                        clearBacklog();
                        _acceptor.close();
                    }
                    else
                    {
                        _threadPool.unregister(_acceptor.fd());
                    }
                }

                java.util.ListIterator i = _collectors.listIterator();
                while (i.hasNext())
                {
                    Collector c = (Collector)i.next();
                    c.destroy();
                }
                _collectors.clear();

                break;
            }
        }

        _state = state;
    }

    private void
    clearBacklog()
    {
        //
        // Clear listen() backlog properly by accepting all queued
        // connections, and then shutting them down.
        //
        while (true)
        {
            try
            {
                Transceiver transceiver = _acceptor.accept(0);
                Collector collector = new Collector(_instance, _adapter,
                                                    transceiver, _endpoint);
                collector.destroy();
            }
            catch (Ice.LocalException ex)
            {
                break;
            }
        }
    }

    private void
    warning(Ice.LocalException ex)
    {
        if (_warnAboutExceptions)
        {
            String s = "server exception:\n" + ex + '\n' +
                _transceiver.toString();
            // TODO: Stack trace?
            _logger.warning(s);
        }
    }

    private Ice.ObjectAdapter _adapter;
    private Acceptor _acceptor;
    private Transceiver _transceiver;
    private Endpoint _endpoint;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private ThreadPool _threadPool;
    private java.util.LinkedList _collectors = new java.util.LinkedList();
    private int _state;
    private boolean _warnAboutExceptions;
}
