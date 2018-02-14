// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Asynchronous callback base class for Communicator.begin_flushBatchRequests.
 **/
public abstract class Callback_Communicator_flushBatchRequests extends IceInternal.CallbackBase
{
    /**
     * Called when the invocation raises an Ice run-time exception.
     *
     * @param ex The Ice run-time exception raised by the operation.
     **/
    public abstract void exception(LocalException ex);

    /**
     * Called when a queued invocation is sent successfully.
     **/
    public void sent(boolean sentSynchronously)
    {
    }

    @Override
    public final void __completed(AsyncResult __result)
    {
        try
        {
            __result.getCommunicator().end_flushBatchRequests(__result);
        }
        catch(LocalException __ex)
        {
            exception(__ex);
        }
    }

    @Override
    public final void __sent(AsyncResult __result)
    {
        sent(__result.sentSynchronously());
    }

    @Override
    public final boolean __hasSentCallback()
    {
        return true;
    }
}
