//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_CallbackBase extends IceInternal.CallbackBase
{
    public Functional_CallbackBase(boolean responseCb,
                                   Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                   Functional_BoolCallback sentCb)
    {
        CallbackBase.check(responseCb || exceptionCb != null);
        _exceptionCb = exceptionCb;
        _sentCb = sentCb;
    }

    protected Functional_CallbackBase(Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                      Functional_BoolCallback sentCb)
    {
        _exceptionCb = exceptionCb;
        _sentCb = sentCb;
    }

    @Override
    public final void _iceSent(Ice.AsyncResult result)
    {
        if(_sentCb != null)
        {
            _sentCb.apply(result.sentSynchronously());
        }
    }

    @Override
    public final boolean _iceHasSentCallback()
    {
        return _sentCb != null;
    }

    @Override
    public abstract void _iceCompleted(Ice.AsyncResult result);

    protected final Functional_GenericCallback1<Ice.Exception> _exceptionCb;
    protected final Functional_BoolCallback _sentCb;
}
