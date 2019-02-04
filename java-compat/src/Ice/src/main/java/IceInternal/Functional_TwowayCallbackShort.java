//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackShort
    extends Functional_TwowayCallback implements Ice.TwowayCallbackShort
{
    public Functional_TwowayCallbackShort(Functional_ShortCallback responseCb,
                                          Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                          Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        _responseCb = responseCb;
    }

    protected Functional_TwowayCallbackShort(boolean userExceptionCb,
                                             Functional_ShortCallback responseCb,
                                             Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                             Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        _responseCb = responseCb;
    }

    @Override
    public void response(short arg)
    {
        if(_responseCb != null)
        {
            _responseCb.apply(arg);
        }
    }

    final private Functional_ShortCallback _responseCb;
}
