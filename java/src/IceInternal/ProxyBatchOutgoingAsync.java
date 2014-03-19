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
    public ProxyBatchOutgoingAsync(Ice.ObjectPrx prx, String operation, CallbackBase callback)
    {
        super(prx.ice_getCommunicator(), ((Ice.ObjectPrxHelperBase)prx).__reference().getInstance(), operation,
              callback);
        _proxy = (Ice.ObjectPrxHelperBase)prx;
        _observer = ObserverHelper.get(prx, operation);
    }

    public void __send()
    {
        Protocol.checkSupportedProtocol(_proxy.__reference().getProtocol());

        //
        // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
        // requests were queued with the connection, they would be lost without being noticed.
        //
        Ice._ObjectDel delegate = null;
        int cnt = -1; // Don't retry.
        try
        {
            delegate = _proxy.__getDelegate(false);
            int status = delegate.__getRequestHandler().flushAsyncBatchRequests(this);
            if((status & AsyncStatus.Sent) > 0)
            {
                _sentSynchronously = true;
                if((status & AsyncStatus.InvokeSentCallback) > 0)
                {
                    __sent();
                }
            }
        }
        catch(Ice.LocalException __ex)
        {
            cnt = _proxy.__handleException(delegate, __ex, null, cnt, _observer);
        }
    }

    @Override
    public Ice.ObjectPrx getProxy()
    {
        return _proxy;
    }

    private Ice.ObjectPrxHelperBase _proxy;
}
