// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Functional_TwowayCallbackLongUE
    extends Functional_TwowayCallbackLong implements Ice.TwowayCallbackLongUE
{
    public Functional_TwowayCallbackLongUE(
        Functional_LongCallback responseCb, 
        Functional_GenericCallback1<Ice.UserException> userExceptionCb, 
        Functional_GenericCallback1<Ice.LocalException> localExceptionCb, 
        Functional_BoolCallback sentCb)
    {
        super(responseCb, localExceptionCb, sentCb);
        __userExceptionCb = userExceptionCb;
    }

    public void exception(Ice.UserException ex)
    {
        if(__userExceptionCb != null)
        {
            __userExceptionCb.apply(ex);
        }
    }
    
    private final Functional_GenericCallback1<Ice.UserException> __userExceptionCb;
};
