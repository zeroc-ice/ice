// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Asynchronous callback base class for Connection.begin_flushBatchRequests.
 **/
public abstract class Callback_Connection_flushBatchRequests extends IceInternal.CallbackBase
{
    /**
     * Called when the invocation raises an Ice run-time exception.
     *
     * @param ex The Ice run-time exception raised by the operation.
     **/
    public abstract void exception(LocalException ex);

    /**
     * Called when a queued invocation is sent successfully.
     *
     * @param sentSynchronously True if the batch request could be flushed synchronously, false otherwise.
     **/
    public void sent(boolean sentSynchronously)
    {
    }

    @Override
    public final void _iceCompleted(AsyncResult result)
    {
        try
        {
            result.getConnection().end_flushBatchRequests(result);
        }
        catch(LocalException ex)
        {
            exception(ex);
        }
    }

    @Override
    public final void _iceSent(AsyncResult result)
    {
        sent(result.sentSynchronously());
    }

    @Override
    public final boolean _iceHasSentCallback()
    {
        return true;
    }
}
