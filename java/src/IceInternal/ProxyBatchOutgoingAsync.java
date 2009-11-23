// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
        super(((Ice.ObjectPrxHelperBase)prx).__reference().getInstance(), operation, callback);
        _proxy = prx;
    }

    public Ice.ObjectPrx getProxy()
    {
        return _proxy;
    }

    private Ice.ObjectPrx _proxy;
}
