// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class AsyncResult
{
    protected AsyncResult(IceInternal.Instance instance, String op, IceInternal.CallbackBase del)
    {
        _instance = instance;
        _operation = op;
        _is = new IceInternal.BasicStream(instance);
        _os = new IceInternal.BasicStream(instance);
        _state = 0;
        _exception = null;
        _callback = del;
    }

    public Communicator getCommunicator()
    {
        return null;
    }

    public Connection getConnection()
    {
        return null;
    }

    public ObjectPrx getProxy()
    {
        return null;
    }

    public final boolean isCompleted()
    {
        synchronized(_monitor)
        {
            return (_state & Done) > 0;
        }
    }

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

    public final boolean isSent()
    {
        synchronized(_monitor)
        {
            return (_state & Sent) > 0;
        }
    }

    public final void waitForSent()
    {
        synchronized(_monitor)
        {
            while((_state & (Sent | Done)) == 0)
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

    public final boolean sentSynchronously()
    {
        return _sentSynchronously; // No lock needed, immutable once __send() is called
    }

    public final String getOperation()
    {
        return _operation;
    }

    public final IceInternal.BasicStream __os()
    {
        return _os;
    }

    public final IceInternal.BasicStream __is()
    {
        return _is;
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
            _is.throwException();
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
            _exception = ex;
            _monitor.notifyAll();
        }

        if(_callback != null)
        {
            try
            {
                _callback.__completed(this);
            }
            catch(RuntimeException exc)
            {
                __warning(exc);
            }
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
            try
            {
                _callback.__sent(this);
            }
            catch(RuntimeException ex)
            {
                __warning(ex);
            }
        }
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
            try
            {
                _callback.__completed(this);
            }
            catch(RuntimeException ex)
            {
                __warning(ex);
            }
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

    private IceInternal.CallbackBase _callback;
}
