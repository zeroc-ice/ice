// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallback extends IceInternal.Functional_CallbackBase implements Ice.TwowayCallback
{
    public Functional_TwowayCallback(boolean responseCb,
                                     Functional_GenericCallback1<Ice.LocalException> localExceptionCb,
                                     Functional_BoolCallback sentCb)
    {
        super(responseCb, localExceptionCb, sentCb);
    }
    
    protected Functional_TwowayCallback(Functional_GenericCallback1<Ice.LocalException> localExceptionCb,
                                        Functional_BoolCallback sentCb)
    {
        super(localExceptionCb, sentCb);
    }
    
    public final void exception(Ice.LocalException ex)
    {
        if(__localExceptionCb != null)
        {
            __localExceptionCb.apply(ex);
        }
    }
}
