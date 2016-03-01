// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackDouble
    extends Functional_TwowayCallback implements Ice.TwowayCallbackDouble
{
    public Functional_TwowayCallbackDouble(Functional_DoubleCallback responseCb,
                                           Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                           Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        __responseCb = responseCb;
    }

    protected Functional_TwowayCallbackDouble(boolean userExceptionCb,
                                              Functional_DoubleCallback responseCb,
                                              Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                              Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        __responseCb = responseCb;
    }

    public void response(double arg)
    {
        if(__responseCb != null)
        {
            __responseCb.apply(arg);
        }
    }

    final private Functional_DoubleCallback __responseCb;
}
