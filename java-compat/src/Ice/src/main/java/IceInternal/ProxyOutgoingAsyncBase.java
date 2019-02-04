//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

//
// Base class for proxy based invocations. This class handles the
// retry for proxy invocations. It also ensures the child observer is
// correct notified of failures and make sure the retry task is
// correctly canceled when the invocation completes.
//
public abstract class ProxyOutgoingAsyncBase extends OutgoingAsyncBase
{
    public static ProxyOutgoingAsyncBase check(Ice.AsyncResult r, Ice.ObjectPrx prx, String operation)
    {
        ProxyOutgoingAsyncBase.checkImpl(r, prx, operation);
        try
        {
            return (ProxyOutgoingAsyncBase)r;
        }
        catch(ClassCastException ex)
        {
            throw new IllegalArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
        }
    }

    public abstract int invokeRemote(Ice.ConnectionI con, boolean compress, boolean response) throws RetryException;

    public abstract int invokeCollocated(CollocatedRequestHandler handler);

    @Override
    public Ice.ObjectPrx getProxy()
    {
        return _proxy;
    }

    @Override
    public boolean completed(Ice.Exception exc)
    {
        if(_childObserver != null)
        {
            _childObserver.failed(exc.ice_id());
            _childObserver.detach();
            _childObserver = null;
        }

        //
        // NOTE: at this point, synchronization isn't needed, no other threads should be
        // calling on the callback.
        //
        try
        {
            //
            // It's important to let the retry queue do the retry even if
            // the retry interval is 0. This method can be called with the
            // connection locked so we can't just retry here.
            //
            _instance.retryQueue().add(this, handleException(exc));
            return false;
        }
        catch(Ice.Exception ex)
        {
            return finished(ex); // No retries, we're done
        }
    }

    public void retryException(Ice.Exception ex)
    {
        try
        {
            //
            // It's important to let the retry queue do the retry. This is
            // called from the connect request handler and the retry might
            // require could end up waiting for the flush of the
            // connection to be done.
            //
            _proxy._updateRequestHandler(_handler, null); // Clear request handler and always retry.
            _instance.retryQueue().add(this, 0);
        }
        catch(Ice.Exception exc)
        {
            if(completed(exc))
            {
                invokeCompletedAsync();
            }
        }
    }

    public void retry()
    {
        invokeImpl(false);
    }

    public void cancelable(final CancellationHandler handler)
    {
        if(_proxy._getReference().getInvocationTimeout() == -2 && _cachedConnection != null)
        {
            final int timeout = _cachedConnection.timeout();
            if(timeout > 0)
            {
                _future = _instance.timer().schedule(
                    new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            cancel(new Ice.ConnectionTimeoutException());
                        }
                    }, timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
            }
        }
        super.cancelable(handler);
    }

    public void abort(Ice.Exception ex)
    {
        assert(_childObserver == null);
        if(finished(ex))
        {
            invokeCompletedAsync();
        }
        else if(ex instanceof Ice.CommunicatorDestroyedException)
        {
            //
            // If it's a communicator destroyed exception, don't swallow
            // it but instead notify the user thread. Even if no callback
            // was provided.
            //
            throw ex;
        }
    }

    protected ProxyOutgoingAsyncBase(Ice.ObjectPrxHelperBase prx, String op, CallbackBase delegate)
    {
        super(prx.ice_getCommunicator(), prx._getReference().getInstance(), op, delegate);
        _proxy = prx;
        _mode = Ice.OperationMode.Normal;
        _cnt = 0;
        _sent = false;
    }

    protected ProxyOutgoingAsyncBase(Ice.ObjectPrxHelperBase prx, String op, CallbackBase delegate, Ice.OutputStream os)
    {
        super(prx.ice_getCommunicator(), prx._getReference().getInstance(), op, delegate, os);
        _proxy = prx;
        _mode = Ice.OperationMode.Normal;
        _cnt = 0;
        _sent = false;
    }

    protected static Ice.AsyncResult checkImpl(Ice.AsyncResult r, Ice.ObjectPrx p, String operation)
    {
        check(r, operation);
        if(r.getProxy() != p)
        {
            throw new IllegalArgumentException("Proxy for call to end_" + operation +
                                               " does not match proxy that was used to call corresponding " +
                                               "begin_" + operation + " method");
        }
        return r;
    }

    protected void invokeImpl(boolean userThread)
    {
        try
        {
            if(userThread)
            {
                int invocationTimeout = _proxy._getReference().getInvocationTimeout();
                if(invocationTimeout > 0)
                {
                    _future = _instance.timer().schedule(
                        new Runnable()
                        {
                            @Override
                            public void run()
                            {
                                cancel(new Ice.InvocationTimeoutException());
                            }
                        }, invocationTimeout, java.util.concurrent.TimeUnit.MILLISECONDS);
                }
            }
            else // If not called from the user thread, it's called from the retry queue
            {
                if(_observer != null)
                {
                    _observer.retried();
                }
            }

            while(true)
            {
                try
                {
                    _sent = false;
                    _handler = null;
                    _handler = _proxy._getRequestHandler();
                    int status = _handler.sendAsyncRequest(this);
                    if((status & AsyncStatus.Sent) > 0)
                    {
                        if(userThread)
                        {
                            _sentSynchronously = true;
                            if((status & AsyncStatus.InvokeSentCallback) > 0)
                            {
                                invokeSent(); // Call the sent callback from the user thread.
                            }
                        }
                        else
                        {
                            if((status & AsyncStatus.InvokeSentCallback) > 0)
                            {
                                invokeSentAsync(); // Call the sent callback from a client thread pool thread.
                            }
                        }
                    }
                    return; // We're done!
                }
                catch(RetryException ex)
                {
                    _proxy._updateRequestHandler(_handler, null); // Clear request handler and always retry.
                }
                catch(Ice.Exception ex)
                {
                    if(_childObserver != null)
                    {
                        _childObserver.failed(ex.ice_id());
                        _childObserver.detach();
                        _childObserver = null;
                    }
                    final int interval = handleException(ex);
                    if(interval > 0)
                    {
                        _instance.retryQueue().add(this, interval);
                        return;
                    }
                    else if(_observer != null)
                    {
                        _observer.retried();
                    }
                }
            }
        }
        catch(Ice.Exception ex)
        {
            //
            // If called from the user thread we re-throw, the exception
            // will be catch by the caller and abort() will be called.
            //
            if(userThread)
            {
                throw ex;
            }
            else if(finished(ex)) // No retries, we're done
            {
                invokeCompletedAsync();
            }
        }
    }

    @Override
    protected boolean sent(boolean done)
    {
        _sent = true;
        if(done)
        {
            if(_future != null)
            {
                _future.cancel(false);
                _future = null;
            }
        }
        return super.sent(done);
    }

    @Override
    protected boolean finished(Ice.Exception ex)
    {
        if(_future != null)
        {
            _future.cancel(false);
            _future = null;
        }
        return super.finished(ex);
    }

    @Override
    protected boolean finished(boolean ok, boolean invoke)
    {
        if(_future != null)
        {
            _future.cancel(false);
            _future = null;
        }
        return super.finished(ok, invoke);
    }

    protected int handleException(Ice.Exception exc)
    {
        Ice.Holder<Integer> interval = new Ice.Holder<Integer>();
        _cnt = _proxy._handleException(exc, _handler, _mode, _sent, interval, _cnt);
        return interval.value;
    }

    final protected Ice.ObjectPrxHelperBase _proxy;
    protected RequestHandler _handler;
    protected Ice.OperationMode _mode;

    private java.util.concurrent.Future<?> _future;
    private int _cnt;
    private boolean _sent;
}
