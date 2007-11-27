// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class AMI_Object_ice_flushBatchRequests extends IceInternal.BatchOutgoingAsync
{
    public abstract void ice_exception(LocalException ex);

    public final void __invoke(Ice.ObjectPrx prx)
    {
        Ice._ObjectDel delegate;
        IceInternal.RequestHandler handler;
        try
        {
            Ice.ObjectPrxHelperBase proxy = (Ice.ObjectPrxHelperBase)prx;
            __prepare(proxy.__reference().getInstance());
            delegate = proxy.__getDelegate(true);
            handler = delegate.__getRequestHandler();
        }
        catch(Ice.LocalException ex)
        {
            __finished(ex);
            return;
        }

        handler.flushAsyncBatchRequests(this);
    }
}
