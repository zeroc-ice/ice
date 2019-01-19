//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackInt
    extends Functional_TwowayCallback implements Ice.TwowayCallbackInt
{
    public Functional_TwowayCallbackInt(Functional_IntCallback responseCb,
                                        Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                        Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        _responseCb = responseCb;
    }

    protected Functional_TwowayCallbackInt(boolean userExceptionCb,
                                           Functional_IntCallback responseCb,
                                           Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                           Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        _responseCb = responseCb;
    }

    @Override
    public void response(int arg)
    {
        if(_responseCb != null)
        {
            _responseCb.apply(arg);
        }
    }

    final private Functional_IntCallback _responseCb;
}
