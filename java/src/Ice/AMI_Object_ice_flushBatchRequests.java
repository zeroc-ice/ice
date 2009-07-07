// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback object for {@link ObjectPrx#.ice_flushBatchRequests_async}.
 **/
public abstract class AMI_Object_ice_flushBatchRequests extends IceInternal.BatchOutgoingAsync
{
    /**
     * Indicates to the caller that a call to <code>ice_flushBatchRequests_async</code>
     * raised an Ice run-time exception.
     *
     * @param ex The run-time exception that was raised.
     *
     * @see ObjectPrx#ice_flushBatchRequests_async
     **/
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
            Ice.ObjectPrxHelperBase proxy = (Ice.ObjectPrxHelperBase)prx;
            try
            {
                delegate = proxy.__getDelegate(true);
                return delegate.__getRequestHandler().flushAsyncBatchRequests(this);
            }
            catch(Ice.LocalException ex)
            {
                proxy.__handleException(delegate, ex, null, -1); // Don't retry
            }
        }
        catch(Ice.LocalException ex)
        {
            __releaseCallback(ex);
        }
        return false;
    }
}
