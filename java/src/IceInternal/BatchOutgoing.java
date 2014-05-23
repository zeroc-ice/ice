// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.Instrumentation.Observer;
import Ice.Instrumentation.InvocationObserver;

public final class BatchOutgoing implements OutgoingMessageCallback
{
    public
    BatchOutgoing(Ice.ConnectionI connection, Instance instance, InvocationObserver observer)
    {
        _connection = connection;
        _sent = false;
        _os = new BasicStream(instance, Protocol.currentProtocolEncoding);
        _observer = observer;
    }

    public
    BatchOutgoing(RequestHandler handler, InvocationObserver observer)
    {
        _handler = handler;
        _sent = false;
        _os = new BasicStream(handler.getReference().getInstance(), Protocol.currentProtocolEncoding);
        _observer = observer;
        Protocol.checkSupportedProtocol(_handler.getReference().getProtocol());
    }

    public void
    invoke()
    {
        assert(_handler != null || _connection != null);

        int timeout;
        if(_connection != null)
        {
            if(_connection.flushBatchRequests(this))
            {
                return;
            }
            timeout = -1;
        }
        else
        {
            try
            {
                if(_handler.sendRequest(this))
                {
                    return;
                }
            }
            catch(IceInternal.LocalExceptionWrapper ex)
            {
                throw ex.get();
            }
            timeout = _handler.getReference().getInvocationTimeout();
        }

        boolean timedOut = false;
        synchronized(this)
        {
            if(timeout > 0)
            {
                long now = Time.currentMonotonicTimeMillis();
                long deadline = now + timeout;
                while(_exception == null && !_sent && !timedOut)
                {
                    try
                    {
                        wait(deadline - now);
                        if(_exception == null && !_sent)
                        {
                            now = Time.currentMonotonicTimeMillis();
                            timedOut = now >= deadline;
                        }
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }
            }
            else
            {
                while(_exception == null && !_sent)
                {
                    try
                    {
                        wait();
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }
            }
        }
        
        if(timedOut)
        {
            _handler.requestTimedOut(this);
            assert(_exception != null);
        }

        if(_exception != null)
        {
            _exception.fillInStackTrace();
            throw _exception;
        }
    }

    public boolean
    send(Ice.ConnectionI connection, boolean compress, boolean response)
    {
        return connection.flushBatchRequests(this);
    }

    synchronized public void
    sent()
    {
        if(_remoteObserver != null)
        {
            _remoteObserver.detach();
            _remoteObserver = null;
        }
        _sent = true;
        notify();
    }
    
    public synchronized void
    finished(Ice.LocalException ex, boolean sent)
    {
        if(_remoteObserver != null)
        {
            _remoteObserver.failed(ex.ice_name());
            _remoteObserver.detach();
            _remoteObserver = null;
        }
        _exception = ex;
        notify();
    }

    public BasicStream
    os()
    {
        return _os;
    }

    public void 
    attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int size)
    {
        if(_observer != null)
        {
            _remoteObserver = _observer.getRemoteObserver(info, endpt, 0, size);
            if(_remoteObserver != null)
            {
                _remoteObserver.attach();
            }
        }
    }

    private RequestHandler _handler;
    private Ice.ConnectionI _connection;
    private BasicStream _os;
    private boolean _sent;
    private Ice.LocalException _exception;

    private InvocationObserver _observer;
    private Observer _remoteObserver;

}
