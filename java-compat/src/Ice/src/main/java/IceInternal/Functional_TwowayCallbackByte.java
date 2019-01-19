//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackByte extends Functional_TwowayCallback implements Ice.TwowayCallbackByte
{
    public Functional_TwowayCallbackByte(Functional_ByteCallback responseCb,
                                         Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                         Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        _responseCb = responseCb;
    }

    protected Functional_TwowayCallbackByte(boolean userExceptionCb,
                                            Functional_ByteCallback responseCb,
                                            Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                            Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        _responseCb = responseCb;
    }

    @Override
    public void response(byte arg)
    {
        if(_responseCb != null)
        {
            _responseCb.apply(arg);
        }
    }

    final private Functional_ByteCallback _responseCb;
}
