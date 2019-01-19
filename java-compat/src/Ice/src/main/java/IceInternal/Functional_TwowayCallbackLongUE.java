//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackLongUE
    extends Functional_TwowayCallbackLong implements Ice.TwowayCallbackLongUE
{
    public Functional_TwowayCallbackLongUE(
        Functional_LongCallback responseCb,
        Functional_GenericCallback1<Ice.UserException> userExceptionCb,
        Functional_GenericCallback1<Ice.Exception> exceptionCb,
        Functional_BoolCallback sentCb)
    {
        super(responseCb, exceptionCb, sentCb);
        _userExceptionCb = userExceptionCb;
    }

    @Override
    public void exception(Ice.UserException ex)
    {
        if(_userExceptionCb != null)
        {
            _userExceptionCb.apply(ex);
        }
    }

    private final Functional_GenericCallback1<Ice.UserException> _userExceptionCb;
}
