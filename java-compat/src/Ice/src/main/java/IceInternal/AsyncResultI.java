// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

public class AsyncResultI implements AsyncResult
{
    @Override
    public void cancel()
    {
        cancel(new Ice.InvocationCanceledException());
    }

    @Override
    public Communicator getCommunicator()
    {
        return _communicator;
    }

    @Override
    public Connection getConnection()
    {
        return null;
    }

    @Override
    public Ice.ObjectPrx getProxy()
    {
        return null;
    }

    @Override
    public final boolean isCompleted()
    {
        synchronized(this)
        {
            return (_state & StateDone) > 0;
        }
    }

    @Override
    public final void waitForCompleted()
    {
        synchronized(this)
        {
            if(Thread.interrupted())
            {
                throw new Ice.OperationInterruptedException();
            }
            while((_state & StateDone) == 0)
            {
                try
                {
                    this.wait();
                }
                catch(InterruptedException ex)
                {
                    throw new Ice.OperationInterruptedException();
                }
            }
        }
    }

    @Override
    public final boolean isSent()
    {
        synchronized(this)
        {
            return (_state & StateSent) > 0;
        }
    }

    @Override
    public final void waitForSent()
    {
        synchronized(this)
        {
            if(Thread.interrupted())
            {
                throw new Ice.OperationInterruptedException();
            }
            while((_state & StateSent) == 0 && _exception == null)
            {
                try
                {
                    this.wait();
                }
                catch(InterruptedException ex)
                {
                    throw new Ice.OperationInterruptedException();
                }
            }
        }
    }

    @Override
    public final void throwLocalException()
    {
        synchronized(this)
        {
            if(_exception != null)
            {
                throw _exception;
            }
        }
    }

    @Override
    public final boolean sentSynchronously()
    {
        return _sentSynchronously; // No lock needed, immutable
    }

    @Override
    public final String getOperation()
    {
        return _operation;
    }

    public final void invokeSent()
    {
        assert(_callback != null);

        if(_instance.useApplicationClassLoader())
        {
            Thread.currentThread().setContextClassLoader(_callback.getClass().getClassLoader());
        }

        try
        {
            _callback._iceSent(this);
        }
        catch(java.lang.RuntimeException ex)
        {
            warning(ex);
        }
        catch(java.lang.Error exc)
        {
            error(exc);
            if(!(exc instanceof java.lang.AssertionError || exc instanceof java.lang.OutOfMemoryError))
            {
                throw exc;
            }
        }
        finally
        {
            if(_instance.useApplicationClassLoader())
            {
                Thread.currentThread().setContextClassLoader(null);
            }
        }

        if(_observer != null)
        {
            Ice.ObjectPrx proxy = getProxy();
            if(proxy == null || !proxy.ice_isTwoway())
            {
                _observer.detach();
                _observer = null;
            }
        }
    }

    public final void invokeCompleted()
    {
        assert(_callback != null);

        if(_instance.useApplicationClassLoader())
        {
            Thread.currentThread().setContextClassLoader(_callback.getClass().getClassLoader());
        }

        try
        {
            _callback._iceCompleted(this);
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

        if(_observer != null)
        {
            _observer.detach();
            _observer = null;
        }
    }

    public final void invokeCompletedAsync()
    {
        //
        // CommunicatorDestroyedCompleted is the only exception that can propagate directly
        // from this method.
        //
        _instance.clientThreadPool().dispatch(new DispatchWorkItem(_cachedConnection)
                                              {
                                                  @Override
                                                  public void run()
                                                  {
                                                      invokeCompleted();
                                                  }
                                              });
    }

    synchronized public void cancelable(final CancellationHandler handler)
    {
        if(_cancellationException != null)
        {
            try
            {
                throw _cancellationException;
            }
            finally
            {
                _cancellationException = null;
            }
        }
        _cancellationHandler = handler;
    }

    public final boolean waitForResponseOrUserEx()
    {
        try
        {
            synchronized(this)
            {
                if((_state & StateEndCalled) > 0)
                {
                    throw new IllegalArgumentException("end_ method called more than once");
                }

                _state |= StateEndCalled;
                if(Thread.interrupted())
                {
                    throw new InterruptedException();
                }
                while((_state & StateDone) == 0)
                {
                    this.wait();
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
            Ice.OperationInterruptedException exc = new Ice.OperationInterruptedException();
            cancel(exc); // Must be called outside the synchronization
            throw exc;
        }
    }

    public void cacheMessageBuffers()
    {
    }

    protected AsyncResultI(Communicator communicator, Instance instance, String op, CallbackBase del)
    {
        _communicator = communicator;
        _instance = instance;
        _operation = op;
        _state = 0;
        _sentSynchronously = false;
        _exception = null;
        _callback = del;
    }

    protected boolean sent(boolean done)
    {
        synchronized(this)
        {
            assert(_exception == null);

            boolean alreadySent = (_state & StateSent) != 0;
            _state |= StateSent;
            if(done)
            {
                _state |= StateDone | StateOK;
                _cancellationHandler = null;
                if(_observer != null && (_callback == null || !_callback._iceHasSentCallback()))
                {
                    _observer.detach();
                    _observer = null;
                }

                //
                // For oneway requests after the data has been sent
                // the buffers can be reused unless this is a
                // collocated invocation. For collocated invocations
                // the buffer won't be reused because it has already
                // been marked as cached in invokeCollocated.
                //
                cacheMessageBuffers();
            }
            this.notifyAll();
            return !alreadySent && _callback != null && _callback._iceHasSentCallback();
        }
    }

    protected boolean finished(boolean ok, boolean invoke)
    {
        synchronized(this)
        {
            _state |= StateDone;
            if(ok)
            {
                _state |= StateOK;
            }
            _cancellationHandler = null;
            invoke &= _callback != null;
            if(!invoke)
            {
                if(_observer != null)
                {
                    _observer.detach();
                    _observer = null;
                }
            }
            this.notifyAll();
            return invoke;
        }
    }

    protected boolean finished(Ice.Exception ex)
    {
        synchronized(this)
        {
            _state |= StateDone;
            _exception = ex;
            _cancellationHandler = null;
            if(_observer != null)
            {
                _observer.failed(ex.ice_id());
            }
            if(_callback == null)
            {
                if(_observer != null)
                {
                    _observer.detach();
                    _observer = null;
                }
            }
            this.notifyAll();
            return _callback != null;
        }
    }

    protected final void invokeSentAsync()
    {
        //
        // This is called when it's not safe to call the sent callback
        // synchronously from this thread. Instead the exception callback
        // is called asynchronously from the client thread pool.
        //
        try
        {
            _instance.clientThreadPool().dispatch(new DispatchWorkItem(_cachedConnection)
                                                  {
                                                      @Override
                                                      public void run()
                                                      {
                                                          invokeSent();
                                                      }
                                                  });
        }
        catch(CommunicatorDestroyedException exc)
        {
        }
    }

    protected void cancel(Ice.LocalException ex)
    {
        CancellationHandler handler;
        synchronized(this)
        {
            _cancellationException = ex;
            if(_cancellationHandler == null)
            {
                return;
            }
            handler = _cancellationHandler;
        }
        handler.asyncRequestCanceled((OutgoingAsyncBase)this, ex);
    }

    protected Ice.Instrumentation.InvocationObserver getObserver()
    {
        return _observer;
    }

    protected static void check(AsyncResult r, String operation)
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

    private void warning(RuntimeException ex)
    {
        if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
        {
            String s = "exception raised by AMI callback:\n" + Ex.toString(ex);
            _instance.initializationData().logger.warning(s);
        }
    }

    private void error(Error error)
    {
        String s = "error raised by AMI callback:\n" + Ex.toString(error);
        _instance.initializationData().logger.error(s);
    }

    protected final Instance _instance;
    protected Ice.Instrumentation.InvocationObserver _observer;
    protected Connection _cachedConnection;
    protected boolean _sentSynchronously;

    private final Communicator _communicator;
    private final String _operation;
    private final CallbackBase _callback;

    private Ice.Exception _exception;

    private CancellationHandler _cancellationHandler;
    private Ice.LocalException _cancellationException;

    protected static final byte StateOK = 0x1;
    protected static final byte StateDone = 0x2;
    protected static final byte StateSent = 0x4;
    protected static final byte StateEndCalled = 0x8;
    protected static final byte StateCachedBuffers = 0x10;
    protected byte _state;
}
