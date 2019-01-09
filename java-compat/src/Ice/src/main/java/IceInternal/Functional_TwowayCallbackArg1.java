// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
        _responseCb = responseCb;
    }

    protected Functional_TwowayCallbackArg1(boolean userExceptionCb,
                                            Functional_GenericCallback1<T> responseCb,
                                            Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                            Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        _responseCb = responseCb;
    }

    @Override
    public void response(T arg)
    {
        if(_responseCb != null)
        {
            _responseCb.apply(arg);
        }
    }

    final private Functional_GenericCallback1<T> _responseCb;
}
