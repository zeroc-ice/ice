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
 * Base class for generated twoway operation callback.
 **/
public abstract class TwowayCallback extends IceInternal.CallbackBase
{
    /**
     * Called when the invocation raises an Ice run-time exception.
     *
     * @param __ex The Ice run-time exception raised by the operation.
     **/
    public abstract void exception(LocalException __ex);

    /**
     * Called when a queued invocation is sent successfully.
     **/
    public void sent(boolean sentSynchronously)
    {
    }

    public final void __sent(AsyncResult __result)
    {
        sent(__result.sentSynchronously());
    }
}
