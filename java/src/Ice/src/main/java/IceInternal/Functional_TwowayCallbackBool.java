// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackBool extends Functional_TwowayCallback implements Ice.TwowayCallbackBool
{
    public Functional_TwowayCallbackBool(Functional_BoolCallback responseCb,
                                         Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                         Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        this.__responseCb = responseCb;
    }

    protected Functional_TwowayCallbackBool(boolean userExceptionCb,
                                            Functional_BoolCallback responseCb,
                                            Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                            Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        this.__responseCb = responseCb;
    }

    @Override
    public void response(boolean arg)
    {
        if(__responseCb != null)
        {
            __responseCb.apply(arg);
        }
    }

    final private Functional_BoolCallback __responseCb;
}
