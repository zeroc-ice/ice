// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class TwowayCallback extends CallbackBase implements Ice.TwowayCallback
{
    public void sent(boolean sentSynchronously)
    {
    }

    @Override
    public void exception(Ice.SystemException __ex)
    {
        exception(new Ice.UnknownException(__ex));
    }

    @Override
    public final void __sent(Ice.AsyncResult __result)
    {
        sent(__result.sentSynchronously());
    }

    @Override
    public final boolean __hasSentCallback()
    {
        return true;
    }
}
