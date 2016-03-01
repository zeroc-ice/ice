// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackShort
    extends Functional_TwowayCallback implements Ice.TwowayCallbackShort
{
    public Functional_TwowayCallbackShort(Functional_ShortCallback responseCb,
                                          Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                          Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        __responseCb = responseCb;
    }

    protected Functional_TwowayCallbackShort(boolean userExceptionCb,
                                             Functional_ShortCallback responseCb,
                                             Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                             Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        __responseCb = responseCb;
    }

    @Override
    public void response(short arg)
    {
        if(__responseCb != null)
        {
            __responseCb.apply(arg);
        }
    }

    final private Functional_ShortCallback __responseCb;
}
