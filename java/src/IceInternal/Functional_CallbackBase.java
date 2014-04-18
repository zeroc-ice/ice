// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_CallbackBase extends IceInternal.CallbackBase
{
    public Functional_CallbackBase(boolean responseCb,
                                   Functional_GenericCallback1<Ice.LocalException> localExceptionCb,
                                   Functional_BoolCallback sentCb)
    {
        CallbackBase.check(responseCb || localExceptionCb != null);
        __localExceptionCb = localExceptionCb;
        __sentCb = sentCb;
    }
    
    protected Functional_CallbackBase(Functional_GenericCallback1<Ice.LocalException> localExceptionCb,
                                      Functional_BoolCallback sentCb)
    {
        __localExceptionCb = localExceptionCb;
        __sentCb = sentCb;
    }
    
    public final void __sent(Ice.AsyncResult __result)
    {
        if(__sentCb != null)
        {
            __sentCb.apply(__result.sentSynchronously());
        }
    }

    public abstract void __completed(Ice.AsyncResult __result);
    
    protected final Functional_GenericCallback1<Ice.LocalException> __localExceptionCb;
    protected final Functional_BoolCallback __sentCb;
}
