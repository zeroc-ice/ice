//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackShortUE
    extends Functional_TwowayCallbackShort implements Ice.TwowayCallbackShortUE
{
    public Functional_TwowayCallbackShortUE(
        Functional_ShortCallback responseCb,
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
