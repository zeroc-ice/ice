//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackFloat
    extends Functional_TwowayCallback implements Ice.TwowayCallbackFloat
{
    public Functional_TwowayCallbackFloat(Functional_FloatCallback responseCb,
                                          Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                          Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        _responseCb = responseCb;
    }

    protected Functional_TwowayCallbackFloat(boolean userExceptionCb,
                                             Functional_FloatCallback responseCb,
                                             Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                             Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        _responseCb = responseCb;
    }

    @Override
    public void response(float arg)
    {
        if(_responseCb != null)
        {
            _responseCb.apply(arg);
        }
    }

    final private Functional_FloatCallback _responseCb;
}
