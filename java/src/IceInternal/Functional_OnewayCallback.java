// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class Functional_OnewayCallback extends IceInternal.Functional_CallbackBase
{
    public Functional_OnewayCallback(Functional_VoidCallback responseCb, 
                                     Functional_GenericCallback1<Ice.LocalException> localExceptionCb,
                                     Functional_BoolCallback sentCb)
    {
        super(localExceptionCb, sentCb);
        CallbackBase.check(responseCb != null || localExceptionCb != null);
        __responseCb = responseCb;
    }

    public final void __completed(Ice.AsyncResult __result)
    {
        try
        {
            ((Ice.ObjectPrxHelperBase)__result.getProxy()).__end(__result, __result.getOperation());
            if(__responseCb != null)
            {
                __responseCb.apply();
            }
        }
        catch(Ice.LocalException __ex)
        {
            if(__localExceptionCb != null)
            {
                __localExceptionCb.apply(__ex);
            }
        }
    }
    
    private final Functional_VoidCallback __responseCb;
}
