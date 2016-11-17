// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Base class for generated oneway operation callback.
 **/
public abstract class OnewayCallback extends IceInternal.CallbackBase
{
    /**
     * Called when the invocation response is received.
     **/
    public abstract void response();

    /**
     * Called when the invocation raises an Ice run-time exception.
     *
     * @param ex The Ice run-time exception raised by the operation.
     **/
    public abstract void exception(LocalException ex);

    /**
     * Called when the invocation raises an Ice system exception.
     *
     * @param ex The Ice system exception raised by the operation.
     **/
    public void exception(SystemException ex)
    {
        exception(new Ice.UnknownException(ex));
    }

    /**
     * Called when a queued invocation is sent successfully.
     *
     * @param sentSynchronously True if the request could be sent synchronously, false otherwise.
     **/
    public void sent(boolean sentSynchronously)
    {
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

    @Override
    public final void _iceCompleted(AsyncResult result)
    {
        try
        {
            ((ObjectPrxHelperBase)result.getProxy())._end(result, result.getOperation());
        }
        catch(LocalException ex)
        {
            exception(ex);
            return;
        }
        catch(SystemException ex)
        {
            exception(ex);
            return;
        }
        response();
    }
}
