// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class GetConnectionOutgoingAsync extends IceInternal.OutgoingAsync
{
    public GetConnectionOutgoingAsync(Ice.ObjectPrxHelperBase prx, String operation, CallbackBase callback)
    {
        super(prx, operation, callback);
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
}
