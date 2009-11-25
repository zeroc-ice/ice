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
 * An application can optionally supply an instance of this class in an
 * asynchronous invocation. The application must create a subclass and
 * implement the exception method.
 **/
public abstract class ExceptionCallback extends Callback
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

    public final void __completed(AsyncResult __result)
    {
        try
        {
            __result.__wait();
        }
        catch(LocalException __ex)
        {
            exception(__ex);
            return;
        }
    }

    public final void __sent(AsyncResult __result)
    {
        sent(__result.sentSynchronously());
    }
}
