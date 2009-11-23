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
 * implement the sent method.
 **/
public abstract class SentCallback extends Callback
{
    /**
     * Called when a queued invocation is sent successfully.
     **/
    public abstract void sent();

    public final void __completed(AsyncResult __result)
    {
        // Ignore.
    }

    public final void __sent(AsyncResult __result)
    {
        sent();
    }
}
