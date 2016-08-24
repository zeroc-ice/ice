// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackUE extends Functional_TwowayCallback implements Ice.TwowayCallbackUE
{
    public Functional_TwowayCallbackUE(boolean responseCb,
                                       Functional_GenericCallback1<Ice.UserException> userExceptionCb,
                                       Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                       Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb || (userExceptionCb != null && exceptionCb != null));
        __userExceptionCb = userExceptionCb;
    }

    @Override
    public final void exception(Ice.UserException ex)
    {
        if(__userExceptionCb != null)
        {
            __userExceptionCb.apply(ex);
        }
    }

    protected final Functional_GenericCallback1<Ice.UserException> __userExceptionCb;
}
