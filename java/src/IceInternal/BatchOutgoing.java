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
    BatchOutgoing(Ice.ConnectionI connection, Instance instance, String op)
    {
        _connection = connection;
        _sent = false;
        _os = new BasicStream(instance, Protocol.currentProtocolEncoding);
        _observer = IceInternal.ObserverHelper.get(instance, op);
    }

    public
    BatchOutgoing(Ice.ObjectPrxHelperBase proxy, String op)
    {
        _proxy = proxy;
        _sent = false;
        _os = new BasicStream(proxy.__reference().getInstance(), Protocol.currentProtocolEncoding);
        _observer = IceInternal.ObserverHelper.get(proxy, op);
        Protocol.checkSupportedProtocol(_proxy.__reference().getProtocol());
    }

    public void
    invoke()
    {
        assert(_proxy != null || _connection != null);

        if(_connection != null)
        {
            if(_connection.flushBatchRequests(this))
            {
                return;
            }

            synchronized(this)
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
                if(_exception != null)
                {
                    throw _exception;
                }
            }
            return;
        }

        RequestHandler handler = null;
        try
        {
            handler = _proxy.__getRequestHandler(false);
            if(handler.sendRequest(this))
            {
                return;
            }

            boolean timedOut = false;
            synchronized(this)
            {
                int timeout = _proxy.__reference().getInvocationTimeout();
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
                handler.requestTimedOut(this);

                synchronized(this)
                {
                    while(_exception == null)
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

            if(_exception != null)
            {
                throw (Ice.Exception)_exception.fillInStackTrace();
            }
        }
        catch(RetryException ex)
        {
            //
            // Clear request handler but don't retry or throw. Retrying
            // isn't useful, there were no batch requests associated with
            // the proxy's request handler.
            //
            _proxy.__setRequestHandler(handler, null); 
        }
        catch(Ice.Exception ex)
        {
            _proxy.__setRequestHandler(handler, null); // Clear request handler
            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }
            throw ex; // Throw to notify the user that batch requests were potentially lost.
        }
    }

    public boolean
    send(Ice.ConnectionI connection, boolean compress, boolean response)
    {
        return connection.flushBatchRequests(this);
    }

    public void
    invokeCollocated(CollocatedRequestHandler handler)
    {
        handler.invokeBatchRequests(this);
    }

    synchronized public void
    sent()
    {
        if(_childObserver != null)
        {
            _childObserver.detach();
            _childObserver = null;
        }
        _sent = true;
        notify();
    }
    
    public synchronized void
    finished(Ice.Exception ex)
    {
        if(_childObserver != null)
        {
            _childObserver.failed(ex.ice_name());
            _childObserver.detach();
            _childObserver = null;
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
            _childObserver = _observer.getRemoteObserver(info, endpt, 0, size);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }

    public void 
    attachCollocatedObserver(Ice.ObjectAdapter adapter, int requestId)
    {
        if(_observer != null)
        {
            _childObserver = _observer.getCollocatedObserver(adapter, requestId, _os.size() - Protocol.headerSize - 4);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }

    private Ice.ObjectPrxHelperBase _proxy;
    private Ice.ConnectionI _connection;
    private BasicStream _os;
    private boolean _sent;
    private Ice.Exception _exception;

    private InvocationObserver _observer;
    private Observer _childObserver;

}
