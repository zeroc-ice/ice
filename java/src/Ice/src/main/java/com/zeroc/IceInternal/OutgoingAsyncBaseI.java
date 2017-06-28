// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

//
// Base class for handling asynchronous invocations. This class is
// responsible for the handling of the output stream and the child
// invocation observer.
//
public abstract class OutgoingAsyncBaseI<T> extends InvocationFutureI<T> implements OutgoingAsyncBase
{
    @Override
    public boolean sent()
    {
        return sent(true);
    }

    @Override
    public boolean completed(com.zeroc.Ice.InputStream is)
    {
        assert(false); // Must be implemented by classes that handle responses
        return false;
    }

    @Override
    public boolean completed(com.zeroc.Ice.Exception ex)
    {
        return finished(ex);
    }

    @Override
    public final void attachRemoteObserver(com.zeroc.Ice.ConnectionInfo info, com.zeroc.Ice.Endpoint endpt,
                                           int requestId)
    {
        if(_observer != null)
        {
            final int size = _os.size() - Protocol.headerSize - 4;
            _childObserver = getObserver().getRemoteObserver(info, endpt, requestId, size);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }

    @Override
    public final void attachCollocatedObserver(com.zeroc.Ice.ObjectAdapter adapter, int requestId)
    {
        if(_observer != null)
        {
            final int size = _os.size() - Protocol.headerSize - 4;
            _childObserver = getObserver().getCollocatedObserver(adapter, requestId, size);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }

    @Override
    public final com.zeroc.Ice.OutputStream getOs()
    {
        return _os;
    }

    public T waitForResponse()
    {
        if(Thread.interrupted())
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }

        try
        {
            return get();
        }
        catch(InterruptedException ex)
        {
            throw new com.zeroc.Ice.OperationInterruptedException(ex);
        }
        catch(java.util.concurrent.ExecutionException ee)
        {
            try
            {
                throw ee.getCause().fillInStackTrace();
            }
            catch(RuntimeException ex) // Includes LocalException
            {
                throw ex;
            }
            catch(Throwable ex)
            {
                throw new com.zeroc.Ice.UnknownException(ex);
            }
        }
    }

    protected OutgoingAsyncBaseI(com.zeroc.Ice.Communicator com, Instance instance, String op)
    {
        super(com, instance, op);
        _os = new com.zeroc.Ice.OutputStream(instance, Protocol.currentProtocolEncoding);
    }

    protected OutgoingAsyncBaseI(com.zeroc.Ice.Communicator com, Instance instance, String op,
                                 com.zeroc.Ice.OutputStream os)
    {
        super(com, instance, op);
        _os = os;
    }

    @Override
    protected boolean sent(boolean done)
    {
        if(done)
        {
            if(_childObserver != null)
            {
                _childObserver.detach();
                _childObserver = null;
            }
        }
        return super.sent(done);
    }

    @Override
    protected boolean finished(com.zeroc.Ice.Exception ex)
    {
        if(_childObserver != null)
        {
            _childObserver.failed(ex.ice_id());
            _childObserver.detach();
            _childObserver = null;
        }
        return super.finished(ex);
    }

    protected com.zeroc.Ice.OutputStream _os;
    protected com.zeroc.Ice.Instrumentation.ChildInvocationObserver _childObserver;
}
