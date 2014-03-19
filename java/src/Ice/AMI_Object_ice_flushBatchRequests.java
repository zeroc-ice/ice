// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback object for {@link ObjectPrx#.ice_flushBatchRequests_async}.
 **/
public abstract class AMI_Object_ice_flushBatchRequests extends Callback_Object_ice_flushBatchRequests
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

    public final void exception(LocalException ex)
    {
        ice_exception(ex);
    }

    @Override
    public final void sent(boolean sentSynchronously)
    {
        if(sentSynchronously && this instanceof AMISentCallback)
        {
            ((AMISentCallback)this).ice_sent();
        }
    }
}
