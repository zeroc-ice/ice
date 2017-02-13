// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class Functional_OnewayCallback extends IceInternal.Functional_CallbackBase
{
    public Functional_OnewayCallback(Functional_VoidCallback responseCb,
                                     Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                     Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || exceptionCb != null);
        __responseCb = responseCb;
    }

    @Override
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
        catch(Ice.Exception __ex)
        {
            if(__exceptionCb != null)
            {
                __exceptionCb.apply(__ex);
            }
        }
    }

    private final Functional_VoidCallback __responseCb;
}
