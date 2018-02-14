// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackArg1<T> extends Functional_TwowayCallback
    implements Ice.TwowayCallbackArg1<T>
{
    public Functional_TwowayCallbackArg1(Functional_GenericCallback1<T> responseCb,
                                         Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                         Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        __responseCb = responseCb;
    }

    protected Functional_TwowayCallbackArg1(boolean userExceptionCb,
                                            Functional_GenericCallback1<T> responseCb,
                                            Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                            Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        __responseCb = responseCb;
    }

    @Override
    public void response(T arg)
    {
        if(__responseCb != null)
        {
            __responseCb.apply(arg);
        }
    }

    final private Functional_GenericCallback1<T> __responseCb;
}
