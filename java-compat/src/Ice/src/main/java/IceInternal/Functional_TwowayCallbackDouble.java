//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackDouble
    extends Functional_TwowayCallback implements Ice.TwowayCallbackDouble
{
    public Functional_TwowayCallbackDouble(Functional_DoubleCallback responseCb,
                                           Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                           Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        _responseCb = responseCb;
    }

    protected Functional_TwowayCallbackDouble(boolean userExceptionCb,
                                              Functional_DoubleCallback responseCb,
                                              Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                              Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        _responseCb = responseCb;
    }

    public void response(double arg)
    {
        if(_responseCb != null)
        {
            _responseCb.apply(arg);
        }
    }

    final private Functional_DoubleCallback _responseCb;
}
