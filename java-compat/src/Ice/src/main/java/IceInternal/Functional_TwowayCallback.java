// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
