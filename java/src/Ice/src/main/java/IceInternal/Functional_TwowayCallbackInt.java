// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackInt
    extends Functional_TwowayCallback implements Ice.TwowayCallbackInt
{
    public Functional_TwowayCallbackInt(Functional_IntCallback responseCb,
                                        Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                        Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        __responseCb = responseCb;
    }

    protected Functional_TwowayCallbackInt(boolean userExceptionCb,
                                           Functional_IntCallback responseCb,
                                           Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                           Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        __responseCb = responseCb;
    }

    @Override
    public void response(int arg)
    {
        if(__responseCb != null)
        {
            __responseCb.apply(arg);
        }
    }

    final private Functional_IntCallback __responseCb;
}
