// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class AMI_Object_ice_flushBatchRequests extends IceInternal.BatchOutgoingAsync
{
    public abstract void ice_exception(LocalException ex);

    public final boolean __invoke(Ice.ObjectPrx prx)
    {
        __acquireCallback(prx);
        try
        {
            //
            // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
            // requests were queued with the connection, they would be lost without being noticed.
            //
            Ice._ObjectDel delegate = null;
            int cnt = -1; // Don't retry.
            Ice.ObjectPrxHelperBase proxy = (Ice.ObjectPrxHelperBase)prx;
            try
            {
                delegate = proxy.__getDelegate(true);
                return delegate.__getRequestHandler().flushAsyncBatchRequests(this);
            }
            catch(Ice.LocalException ex)
            {
                cnt = proxy.__handleException(delegate, ex, null, cnt);
            }
        }
        catch(Ice.LocalException ex)
        {
            __releaseCallback(ex);
        }
        return false;
    }
}
