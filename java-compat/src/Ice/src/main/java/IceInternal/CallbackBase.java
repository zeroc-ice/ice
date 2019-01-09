// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public abstract class CallbackBase
{
    public abstract void _iceCompleted(Ice.AsyncResult r);
    public abstract void _iceSent(Ice.AsyncResult r);
    public abstract boolean _iceHasSentCallback();

    public static void check(boolean cb)
    {
        if(!cb)
        {
            throw new IllegalArgumentException("callback cannot be null");
        }
    }
}
