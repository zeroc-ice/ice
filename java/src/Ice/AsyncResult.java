// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * An AsyncResult object is the return value of an asynchronous invocation.
 * With this object, an application can obtain several attributes of the
 * invocation and discover its outcome.
 **/
public class AsyncResult
{
    protected AsyncResult(Communicator communicator, IceInternal.Instance instance, String op,
                          IceInternal.CallbackBase del)
    {
        _communicator = communicator;
        _instance = instance;
        _operation = op;
        _os = new IceInternal.BasicStream(instance, IceInternal.Protocol.currentProtocolEncoding, false, false);
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
    public Communicator getCommunicator()
    {
        return _communicator;
    }

    /**
     * Returns the connection that was used for the invocation.
     *
     * @return The connection.
     **/
    public Connection getConnection()
    {
        return null;
    }

    /**
     * Returns the proxy that was used to call the <code>begin_</code> method.
     *
     * @return The proxy.
     **/
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
    public final boolean isCompleted()
    {
        synchronized(_monitor)
        {
            return (_state & Done) > 0;
        }
    }

    /**
     * Blocks the caller until the result of the invocation is available.
     **/
    public final void waitForCompleted()
    {
        synchronized(_monitor)
        {
            while((_state & Done) == 0)
            {
                try
                {
                    _monitor.wait();
                }
                catch(InterruptedException ex)
                {
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
    public final boolean isSent()
    {
        synchronized(_monitor)
        {
            return (_state & Sent) > 0;
        }
    }

    /**
     * Blocks the caller until the request has been written to the client-side transport.
     **/
    public final void waitForSent()
    {
        synchronized(_monitor)
        {
            while((_state & Sent) == 0 && _exception == null)
            {
                try
                {
                    _monitor.wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
        }
    }

    /**
     * If the invocation failed with a local exception, throws the local exception.
     **/
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
    public final boolean sentSynchronously()
    {
        return _sentSynchronously; // No lock needed, immutable once __send() is called
    }

    /**
     * Returns the name of the operation.
     *
     * @return The operation name.
     **/
    public final String getOperation()
    {
        return _operation;
    }

    public final IceInternal.BasicStream __getOs()
    {
        return _os;
    }

    public IceInternal.BasicStream
    __startReadParams()
    {
        _is.startReadEncaps();
        return _is;
    }

    public void
    __endReadParams()
    {
        _is.endReadEncaps();
    }

    public void
    __readEmptyParams()
    {
        _is.skipEmptyEncaps();
    }

    public byte[]
    __readParamEncaps()
    {
        return _is.readEncaps(null);
    }

    public final boolean __wait()
    {
        synchronized(_monitor)
        {
            if((_state & EndCalled) > 0)
            {
                throw new java.lang.IllegalArgumentException("end_ method called more than once");
            }
            _state |= EndCalled;
            while((_state & Done) == 0)
            {
                try
                {
                    _monitor.wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
            if(_exception != null)
            {
                //throw (LocalException)_exception.fillInStackTrace();
                throw _exception;
            }
            return (_state & OK) > 0;
        }
    }

    public final void __throwUserException()
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

    public final void __exceptionAsync(final LocalException ex)
    {
        //
        // This is called when it's not safe to call the exception callback synchronously
        // from this thread. Instead the exception callback is called asynchronously from
        // the client thread pool.
        //
        try
        {
            _instance.clientThreadPool().execute(new IceInternal.DispatchWorkItem(_instance)
                {
                    public void
                    run()
                    {
                        __exception(ex);
                    }
                });
        }
        catch(CommunicatorDestroyedException exc)
        {
            throw exc; // CommunicatorDestroyedException is the only exception that can propagate directly.
        }
    }

    public final void __exception(LocalException ex)
    {
        synchronized(_monitor)
        {
            _state |= Done;
            _os.resize(0, false); // Clear buffer now, instead of waiting for AsyncResult deallocation
            _exception = ex;
            _monitor.notifyAll();
        }

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
            catch(RuntimeException exc)
            {
                __warning(exc);
            }
            catch(AssertionError exc)
            {
                __error(exc);
            }
            catch(OutOfMemoryError exc)
            {
                __error(exc);
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

    protected final void __sentInternal()
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
                __warning(ex);
            }
            catch(AssertionError exc)
            {
                __error(exc);
            }
            catch(OutOfMemoryError exc)
            {
                __error(exc);
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

    public void
    __attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId, int size)
    {
        if(_observer != null)
        {
            _remoteObserver = _observer.getRemoteObserver(info, endpt, requestId, size);
            if(_remoteObserver != null)
            {
                _remoteObserver.attach();
            }
        }
    }

    public Ice.Instrumentation.InvocationObserver __getObserver()
    {
        return _observer;
    }

    public final void __sentAsync()
    {
        //
        // This is called when it's not safe to call the sent callback synchronously
        // from this thread. Instead the exception callback is called asynchronously from
        // the client thread pool.
        //
        try
        {
            _instance.clientThreadPool().execute(new IceInternal.DispatchWorkItem(_instance)
                {
                    public void
                    run()
                    {
                        __sentInternal();
                    }
                });
        }
        catch(CommunicatorDestroyedException exc)
        {
        }
    }

    public static void __check(AsyncResult r, ObjectPrx prx, String operation)
    {
        __check(r, operation);
        if(r.getProxy() != prx)
        {
            throw new IllegalArgumentException("Proxy for call to end_" + operation +
                                               " does not match proxy that was used to call corresponding begin_" +
                                               operation + " method");
        }
    }

    public static void __check(AsyncResult r, Connection con, String operation)
    {
        __check(r, operation);
        if(r.getConnection() != con)
        {
            throw new IllegalArgumentException("Connection for call to end_" + operation +
                                               " does not match connection that was used to call corresponding begin_" +
                                               operation + " method");
        }
    }

    public static void __check(AsyncResult r, Communicator com, String operation)
    {
        __check(r, operation);
        if(r.getCommunicator() != com)
        {
            throw new IllegalArgumentException("Communicator for call to end_" + operation +
                                               " does not match communicator that was used to call corresponding " +
                                               "begin_" + operation + " method");
        }
    }

    protected static void __check(AsyncResult r, String operation)
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

    protected final void __response()
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
                __warning(ex);
            }
            catch(AssertionError exc)
            {
                __error(exc);
            }
            catch(OutOfMemoryError exc)
            {
                __error(exc);
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

    protected final void __warning(RuntimeException ex)
    {
        if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
        {
            String s = "exception raised by AMI callback:\n" + IceInternal.Ex.toString(ex);
            _instance.initializationData().logger.warning(s);
        }
    }

    protected final void __error(Error error)
    {
        String s = "error raised by AMI callback:\n" + IceInternal.Ex.toString(error);
        _instance.initializationData().logger.error(s);
    }

    protected Communicator _communicator;
    protected IceInternal.Instance _instance;
    protected String _operation;

    protected java.lang.Object _monitor = new java.lang.Object();
    protected IceInternal.BasicStream _is;
    protected IceInternal.BasicStream _os;

    protected static final byte OK = 0x1;
    protected static final byte Done = 0x2;
    protected static final byte Sent = 0x4;
    protected static final byte EndCalled = 0x8;

    protected byte _state;
    protected boolean _sentSynchronously;
    protected LocalException _exception;

    protected Ice.Instrumentation.InvocationObserver _observer;
    protected Ice.Instrumentation.RemoteObserver _remoteObserver;

    private IceInternal.CallbackBase _callback;
}
