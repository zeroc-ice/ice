// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

        if(_handler != null && !_handler.flushBatchRequests(this) || 
           _connection != null && !_connection.flushBatchRequests(this))
        {
            synchronized(this)
            {
                while(_exception == null && !_sent)
                {
                    try
                    {
                        wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                
                if(_exception != null)
                {
                    throw _exception;
                }
            }
        }
    }

    public void
    sent(boolean async)
    {
        if(async)
        {
            synchronized(this)
            {
                _sent = true;
                notify();
            }
        }
        else
        {
            _sent = true;
        }
        if(_remoteObserver != null)
        {
            _remoteObserver.detach();
            _remoteObserver = null;
        }
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
