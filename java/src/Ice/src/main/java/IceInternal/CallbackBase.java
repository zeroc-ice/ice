// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class CallbackBase
{
    public abstract void __completed(Ice.AsyncResult r);
    public abstract void __sent(Ice.AsyncResult r);
    public abstract boolean __hasSentCallback();

    public static void check(boolean cb)
    {
        if(!cb)
        {
            throw new IllegalArgumentException("callback cannot be null");
        }
    }
}
