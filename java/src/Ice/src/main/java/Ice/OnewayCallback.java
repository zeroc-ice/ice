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
     **/
    public void sent(boolean sentSynchronously)
    {
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

    @Override
    public final void __completed(AsyncResult __result)
    {
        try
        {
            ((ObjectPrxHelperBase)__result.getProxy()).__end(__result, __result.getOperation());
        }
        catch(LocalException __ex)
        {
            exception(__ex);
            return;
        }
        catch(SystemException __ex)
        {
            exception(__ex);
            return;
        }
        response();
    }
}
