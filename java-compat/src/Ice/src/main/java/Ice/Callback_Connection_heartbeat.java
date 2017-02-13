// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Asynchronous callback base class for Connection.begin_heartbeat.
 **/
public abstract class Callback_Connection_heartbeat extends IceInternal.CallbackBase
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
    public final void _iceCompleted(AsyncResult __result)
    {
        try
        {
            __result.getConnection().end_heartbeat(__result);
        }
        catch(LocalException __ex)
        {
            exception(__ex);
        }
    }

    @Override
    public final void _iceSent(AsyncResult __result)
    {
        sent(__result.sentSynchronously());
    }

    @Override
    public final boolean _iceHasSentCallback()
    {
        return true;
    }
}
