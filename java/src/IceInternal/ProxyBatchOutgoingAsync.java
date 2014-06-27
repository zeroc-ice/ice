// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ProxyBatchOutgoingAsync extends BatchOutgoingAsync
{
    public ProxyBatchOutgoingAsync(Ice.ObjectPrxHelperBase prx, String operation, CallbackBase callback)
    {
        super(prx.ice_getCommunicator(), prx.__reference().getInstance(), operation, callback);
        _proxy = prx;
        _observer = ObserverHelper.get(prx, operation);
    }

    public void __invoke()
    {
        Protocol.checkSupportedProtocol(_proxy.__reference().getProtocol());

        RequestHandler handler = null;
        try
        {
            handler = _proxy.__getRequestHandler(true);
            int status = handler.sendAsyncRequest(this);
            if((status & AsyncStatus.Sent) > 0)
            {
                _sentSynchronously = true;
                if((status & AsyncStatus.InvokeSentCallback) > 0)
                {
                    __invokeSent();
                }
            }
            else
            {
                synchronized(_monitor)
                {
                    if((_state & Done) == 0)
                    {
                        int invocationTimeout = handler.getReference().getInvocationTimeout();
                        if(invocationTimeout > 0)
                        {
                            _instance.timer().schedule(this, invocationTimeout);
                            _timeoutRequestHandler = handler;
                        }
                    }
                }
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
            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }
            _proxy.__setRequestHandler(handler, null); // Clear request handler
            throw ex; // Throw to notify the user lthat batch requests were potentially lost.
        }
    }

    @Override
    public Ice.ObjectPrx getProxy()
    {
        return _proxy;
    }

    private Ice.ObjectPrxHelperBase _proxy;
}
