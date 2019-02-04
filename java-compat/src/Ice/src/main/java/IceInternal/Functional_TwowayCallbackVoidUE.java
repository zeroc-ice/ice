//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackVoidUE
    extends Functional_TwowayCallbackUE implements Ice.TwowayCallbackVoidUE
{
    public Functional_TwowayCallbackVoidUE(
        Functional_VoidCallback responseCb,
        Functional_GenericCallback1<Ice.UserException> userExceptionCb,
        Functional_GenericCallback1<Ice.Exception> exceptionCb,
        Functional_BoolCallback sentCb)
    {
        super(responseCb != null, userExceptionCb, exceptionCb, sentCb);
        _responseCb = responseCb;
    }

    @Override
    public void response()
    {
        if(_responseCb != null)
        {
            _responseCb.apply();
        }
    }

    private final Functional_VoidCallback _responseCb;
}
