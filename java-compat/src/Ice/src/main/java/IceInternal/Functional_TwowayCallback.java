// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallback extends IceInternal.Functional_CallbackBase implements Ice.TwowayCallback
{
    public Functional_TwowayCallback(boolean responseCb,
                                     Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                     Functional_BoolCallback sentCb)
    {
        super(responseCb, exceptionCb, sentCb);
    }

    protected Functional_TwowayCallback(Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                        Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
    }

    @Override
    public void exception(Ice.SystemException ex)
    {
        if(_exceptionCb != null)
        {
            _exceptionCb.apply(ex);
        }
    }

    @Override
    public final void exception(Ice.LocalException ex)
    {
        if(_exceptionCb != null)
        {
            _exceptionCb.apply(ex);
        }
    }
}
