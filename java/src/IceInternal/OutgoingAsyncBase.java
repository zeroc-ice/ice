// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.AsyncResult;
import Ice.Communicator;
import Ice.CommunicatorDestroyedException;
import Ice.Connection;
import Ice.ObjectPrx;
import Ice.UserException;

/**
 * An AsyncResult object is the return value of an asynchronous invocation.
 * With this object, an application can obtain several attributes of the
 * invocation and discover its outcome.
 **/
public abstract class OutgoingAsyncBase implements Ice.AsyncResult
{
    protected OutgoingAsyncBase(Communicator communicator, IceInternal.Instance instance, String op,
                                IceInternal.CallbackBase del)
    {
        _communicator = communicator;
        _instance = instance;
        _operation = op;
        _os = new IceInternal.BasicStream(instance, IceInternal.Protocol.currentProtocolEncoding);
        _state = 0;
        _sentSynchronously = false;
        _exception = null;
        _callback = del;
    }

    protected OutgoingAsyncBase(Communicator communicator, Instance instance, String op, CallbackBase del,
                                BasicStream is, BasicStream os)
    {
        _communicator = communicator;
        _instance = instance;
        _operation = op;
        _os = os;
        _is = is;
        _state = 0;
        _sentSynchronously = false;
        _exception = null;
        _callback = del;
    }

    /**
     * Returns the communicator that sent the invocation.
     *
     * @return The communicator.
     **/
    @Override
    public Communicator getCommunicator()
    {
        return _communicator;
    }

    /**
     * Returns the connection that was used for the invocation.
     *
     * @return The connection.
     **/
    @Override
    public Connection getConnection()
    {
        return null;
    }

    /**
     * Returns the proxy that was used to call the <code>begin_</code> method.
     *
     * @return The proxy.
     **/
    @Override
    public ObjectPrx getProxy()
    {
        return null;
    }

    /**
     * Indicates whether the result of an invocation is available.
     *
     * @return True if the result is available, which means a call to the <code>end_</code>
     * method will not block. The method returns false if the result is not yet available.
     **/
    @Override
    public final boolean isCompleted()
    {
        synchronized(_monitor)
        {
            return (_state & StateDone) > 0;
        }
    }

    /**
     * Blocks the caller until the result of the invocation is available.
     **/
    @Override
    public final void waitForCompleted()
    {
        synchronized(_monitor)
        {
            if(Thread.interrupted())
            {
                throw new Ice.OperationInterruptedException();
            }
            while((_state & StateDone) == 0)
            {
                try
                {
                    _monitor.wait();
                }
                catch(InterruptedException ex)
                {
                    throw new Ice.OperationInterruptedException();
                }
            }
        }
    }

    /**
     * When you call the <code>begin_</code> method, the Ice run time attempts to
     * write the corresponding request to the client-side transport. If the
     * transport cannot accept the request, the Ice run time queues the request
     * for later transmission. This method returns true if, at the time it is called,
     * the request has been written to the local transport (whether it was initially
     * queued or not). Otherwise, if the request is still queued, this method returns
     * false.
     *
     * @return True if the request has been sent, or false if the request is queued.
     **/
    @Override
    public final boolean isSent()
    {
        synchronized(_monitor)
        {
            return (_state & StateSent) > 0;
        }
    }

    /**
     * Blocks the caller until the request has been written to the client-side transport.
     **/
    @Override
    public final void waitForSent()
    {
        synchronized(_monitor)
        {
            if(Thread.interrupted())
            {
                throw new Ice.OperationInterruptedException();
            }
            while((_state & StateSent) == 0 && _exception == null)
            {
                try
                {
                    _monitor.wait();
                }
                catch(InterruptedException ex)
                {
                    throw new Ice.OperationInterruptedException();
                }
            }
        }
    }

    /**
     * If the invocation failed with a local exception, throws the local exception.
     **/
    @Override
    public final void throwLocalException()
    {
        synchronized(_monitor)
        {
            if(_exception != null)
            {
                throw _exception;
            }
        }
    }

    /**
     * This method returns true if a request was written to the client-side
     * transport without first being queued. If the request was initially
     * queued, this method returns false (independent of whether the request
     * is still in the queue or has since been written to the client-side transport).
     *
     * @return True if the request was sent without being queued, or false
     * otherwise.
     **/
    @Override
    public final boolean sentSynchronously()
    {
        return _sentSynchronously; // No lock needed, immutable once __send() is called
    }

    /**
     * Returns the name of the operation.
     *
     * @return The operation name.
     **/
    @Override
    public final String getOperation()
    {
        return _operation;
    }

    public final IceInternal.BasicStream getOs()
    {
        return _os;
    }

    public IceInternal.BasicStream
    startReadParams()
    {
        _is.startReadEncaps();
        return _is;
    }

    public void
    endReadParams()
    {
        _is.endReadEncaps();
    }

    public void
    readEmptyParams()
    {
        _is.skipEmptyEncaps(null);
    }

    public byte[]
    readParamEncaps()
    {
        return _is.readEncaps(null);
    }

    public final boolean __wait()
    {
        try 
        {
            synchronized(_monitor)
            {
                if((_state & StateEndCalled) > 0)
                {
                    throw new java.lang.IllegalArgumentException("end_ method called more than once");
                }
    
                _state |= StateEndCalled;
                if(Thread.interrupted())
                {
                    throw new InterruptedException();
                }
                while((_state & StateDone) == 0)
                {
                    _monitor.wait();
                }
    
                if(_exception != null)
                {
                    throw (Ice.Exception)_exception.fillInStackTrace();
                }
    
                return (_state & StateOK) > 0;
            }
        }
        catch(InterruptedException ex)
        {
            // This must be called outside of the monitor as the
            // invocation will potentially want to lock the
            // connection (which in turn may want to lock the outgoing
            // to notify that the message has been sent).
            cancelRequest();
            throw new Ice.OperationInterruptedException();
        }
    }

    public final void throwUserException()
        throws UserException
    {
        try
        {
            _is.startReadEncaps();
            _is.throwException(null);
        }
        catch(UserException ex)
        {
            _is.endReadEncaps();
            throw ex;
        }
    }

    public void invokeExceptionAsync(final Ice.Exception ex)
    {
        //
        // This is called when it's not safe to call the exception callback synchronously
        // from this thread. Instead the exception callback is called asynchronously from
        // the client thread pool.
        //
        try
        {
            _instance.clientThreadPool().dispatch(new IceInternal.DispatchWorkItem(_cachedConnection)
                {
                    @Override
                    public void
                    run()
                    {
                        invokeException(ex);
                    }
                });
        }
        catch(CommunicatorDestroyedException exc)
        {
            throw exc; // CommunicatorDestroyedException is the only exception that can propagate directly.
        }
    }

    public final void invokeException(Ice.Exception ex)
    {
        synchronized(_monitor)
        {
            _state |= StateDone;
            //_os.resize(0, false); // Clear buffer now, instead of waiting for AsyncResult deallocation
            _exception = ex;
            _monitor.notifyAll();
        }

        invokeCompleted();
    }

    protected final void invokeSentInternal()
    {
        //
        // Note: no need to change the _state here, specializations are responsible for
        // changing the state.
        //

        if(_callback != null)
        {
            if(_instance.useApplicationClassLoader())
            {
                Thread.currentThread().setContextClassLoader(_callback.getClass().getClassLoader());
            }

            try
            {
                _callback.__sent(this);
            }
            catch(RuntimeException ex)
            {
                warning(ex);
            }
            catch(AssertionError exc)
            {
                error(exc);
            }
            catch(OutOfMemoryError exc)
            {
                error(exc);
            }
            finally
            {
                if(_instance.useApplicationClassLoader())
                {
                    Thread.currentThread().setContextClassLoader(null);
                }
            }
        }

        if(_observer != null)
        {
            Ice.ObjectPrx proxy = getProxy();
            if(proxy == null || !proxy.ice_isTwoway())
            {
                _observer.detach();
            }
        }
    }

    public void attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId, int size)
    {
        if(_observer != null)
        {
            _childObserver = _observer.getRemoteObserver(info, endpt, requestId, size);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }
    
    void attachCollocatedObserver(Ice.ObjectAdapter adapter, int requestId)
    {
        if(_observer != null)
        {
            _childObserver = _observer.getCollocatedObserver(adapter,
                                                             requestId,
                                                             _os.size() - IceInternal.Protocol.headerSize - 4);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }

    abstract void processRetry();
    
    final protected void invokeSentAsync()
    {
        //
        // This is called when it's not safe to call the sent callback synchronously
        // from this thread. Instead the exception callback is called asynchronously from
        // the client thread pool.
        //
        try
        {
            _instance.clientThreadPool().dispatch(new IceInternal.DispatchWorkItem(_cachedConnection)
            {
                @Override
                public void run()
                {
                    invokeSentInternal();
                }
            });
        }
        catch(CommunicatorDestroyedException exc)
        {
        }
    }

    public static void check(AsyncResult r, ObjectPrx prx, String operation)
    {
        check(r, operation);
        if(r.getProxy() != prx)
        {
            throw new IllegalArgumentException("Proxy for call to end_" + operation +
                                               " does not match proxy that was used to call corresponding begin_" +
                                               operation + " method");
        }
    }

    public static void check(AsyncResult r, Connection con, String operation)
    {
        check(r, operation);
        if(r.getConnection() != con)
        {
            throw new IllegalArgumentException("Connection for call to end_" + operation +
                                               " does not match connection that was used to call corresponding begin_" +
                                               operation + " method");
        }
    }

    public static void check(AsyncResult r, Communicator com, String operation)
    {
        check(r, operation);
        if(r.getCommunicator() != com)
        {
            throw new IllegalArgumentException("Communicator for call to end_" + operation +
                                               " does not match communicator that was used to call corresponding " +
                                               "begin_" + operation + " method");
        }
    }

    public void cacheMessageBuffers()
    {
    }

    public final void invokeCompleted()
    {
        //
        // Note: no need to change the _state here, specializations are responsible for
        // changing the state.
        //

        if(_callback != null)
        {
            if(_instance.useApplicationClassLoader())
            {
                Thread.currentThread().setContextClassLoader(_callback.getClass().getClassLoader());
            }

            try
            {
                _callback.__completed(this);
            }
            catch(RuntimeException ex)
            {
                warning(ex);
            }
            catch(AssertionError exc)
            {
                error(exc);
            }
            catch(OutOfMemoryError exc)
            {
                error(exc);
            }
            finally
            {
                if(_instance.useApplicationClassLoader())
                {
                    Thread.currentThread().setContextClassLoader(null);
                }
            }
        }

        if(_observer != null)
        {
            _observer.detach();
            _observer = null;
        }
    }

    protected void
    timeout()
    {
        IceInternal.RequestHandler handler;
        synchronized(_monitor)
        {
            handler = _timeoutRequestHandler;
            _timeoutRequestHandler = null;
        }

        if(handler != null)
        {
            handler.asyncRequestCanceled((IceInternal.OutgoingAsyncMessageCallback)this, 
                                         new Ice.InvocationTimeoutException());
        }
    }

    private static void check(AsyncResult r, String operation)
    {
        if(r == null)
        {
            throw new IllegalArgumentException("AsyncResult == null");
        }
        else if(r.getOperation() != operation) // Do NOT use equals() here - we are comparing reference equality
        {
            throw new IllegalArgumentException("Incorrect operation for end_" + operation + " method: " +
                                               r.getOperation());
        }
    }

    private final void warning(RuntimeException ex)
    {
        if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
        {
            String s = "exception raised by AMI callback:\n" + IceInternal.Ex.toString(ex);
            _instance.initializationData().logger.warning(s);
        }
    }

    private final void error(Error error)
    {
        String s = "error raised by AMI callback:\n" + IceInternal.Ex.toString(error);
        _instance.initializationData().logger.error(s);
    }
    
    abstract protected void cancelRequest();

    protected Communicator _communicator;
    protected IceInternal.Instance _instance;
    protected String _operation;
    protected Ice.Connection _cachedConnection;

    protected java.lang.Object _monitor = new java.lang.Object();
    protected IceInternal.BasicStream _is;
    protected IceInternal.BasicStream _os;

    protected IceInternal.RequestHandler _timeoutRequestHandler;
    protected java.util.concurrent.Future<?> _future;

    protected static final byte StateOK = 0x1;
    protected static final byte StateDone = 0x2;
    protected static final byte StateSent = 0x4;
    protected static final byte StateEndCalled = 0x8;
    protected static final byte StateCachedBuffers = 0x10;

    protected byte _state;
    protected boolean _sentSynchronously;
    protected Ice.Exception _exception;

    protected Ice.Instrumentation.InvocationObserver _observer;
    protected Ice.Instrumentation.ChildInvocationObserver _childObserver;

    protected IceInternal.CallbackBase _callback;
}
