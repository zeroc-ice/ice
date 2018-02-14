// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackLong
    extends Functional_TwowayCallback implements Ice.TwowayCallbackLong
{
    public Functional_TwowayCallbackLong(Functional_LongCallback responseCb,
                                         Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                         Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        __responseCb = responseCb;
    }

    protected Functional_TwowayCallbackLong(boolean userExceptionCb,
                                            Functional_LongCallback responseCb,
                                            Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                            Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        __responseCb = responseCb;
    }

    @Override
    public void response(long arg)
    {
        if(__responseCb != null)
        {
            __responseCb.apply(arg);
        }
    }

    final private Functional_LongCallback __responseCb;
}
