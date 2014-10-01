// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class GetConnectionOutgoingAsync extends OutgoingAsyncBase implements OutgoingAsyncMessageCallback
{
    public GetConnectionOutgoingAsync(Ice.ObjectPrxHelperBase prx, String operation, CallbackBase cb)
    {
        super(prx.ice_getCommunicator(), ((Ice.ObjectPrxHelperBase) prx).__reference().getInstance(), operation, cb);
        _proxy = (Ice.ObjectPrxHelperBase) prx;
        _cnt = 0;
        _observer = ObserverHelper.get(prx, operation);
    }

    public void __invoke()
    {
        while(true)
        {
            try
            {
                _handler = _proxy.__getRequestHandler();
                _handler.sendAsyncRequest(this);
            }
            catch(RetryException ex)
            {
                _proxy.__setRequestHandler(_handler, null);
            }
            catch(Ice.Exception ex)
            {
                handleException(ex);
            }
            break;
        }
    }

    @Override
    public Ice.ObjectPrx getProxy()
    {
        return _proxy;
    }

    @Override
    public int send(Ice.ConnectionI conection, boolean compress, boolean response)
        throws RetryException
    {
        sent();
        return 0;
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler)
    {
        sent();
        return 0;
    }

    @Override
    public boolean sent()
    {
        synchronized(_monitor)
        {
            _state |= StateDone;
            _monitor.notifyAll();
        }
        invokeCompleted();
        return false;
    }

    @Override
    public void invokeSent()
    {
        // No sent callback
    }

    @Override
    public void finished(Ice.Exception exc)
    {
        try
        {
            handleException(exc);
        }
        catch(Ice.Exception ex)
        {
            invokeExceptionAsync(ex);
        }
    }

    @Override
    void processRetry()
    {
        __invoke();
    }

    @Override
    public void dispatchInvocationCancel(final Ice.LocalException ex, ThreadPool threadPool, Ice.Connection connection)
    {
        threadPool.dispatch(new DispatchWorkItem(connection)
        {
            @Override
            public void run()
            {
                GetConnectionOutgoingAsync.this.finished(ex);
            }
        });
    }

    @Override
    protected void cancelRequest()
    {
        if(_handler != null)
        {
            _handler.asyncRequestCanceled(this, new Ice.OperationInterruptedException());
        }
    }

    private void handleException(Ice.Exception exc)
    {
       try
        {
            Ice.Holder<Integer> interval = new Ice.Holder<Integer>();
            _cnt = _proxy.__handleException(exc, _handler, Ice.OperationMode.Idempotent, false, interval, _cnt);
            if(_observer != null)
            {
                _observer.retried(); // Invocation is being retried
            }
            _instance.retryQueue().add(this, interval.value);
        }
        catch(Ice.Exception ex)
        {
            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }
            throw ex;
        }
    }

    private Ice.ObjectPrxHelperBase _proxy;
    private RequestHandler _handler = null;
    private int _cnt;
}
