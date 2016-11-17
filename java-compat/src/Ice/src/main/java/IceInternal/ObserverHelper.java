// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.Instrumentation.CommunicatorObserver;
import Ice.Instrumentation.InvocationObserver;

public final class ObserverHelper
{
    static public InvocationObserver
    get(Instance instance, String op)
    {
        CommunicatorObserver obsv = instance.initializationData().observer;
        if(obsv != null)
        {
            InvocationObserver observer = obsv.getInvocationObserver(null, op, _emptyContext);
            if(observer != null)
            {
                observer.attach();
            }
            return observer;
        }
        return null;
    }

    static public InvocationObserver
    get(Ice.ObjectPrx proxy, String op)
    {
        return get(proxy, op, null);
    }

    static public InvocationObserver
    get(Ice.ObjectPrx proxy, String op, java.util.Map<String, String> context)
    {
        CommunicatorObserver obsv = 
            ((Ice.ObjectPrxHelperBase)proxy)._getReference().getInstance().initializationData().observer;
        if(obsv != null)
        {
            InvocationObserver observer;
            if(context == null)
            {
                observer = obsv.getInvocationObserver(proxy, op, _emptyContext);
            }
            else
            {
                observer = obsv.getInvocationObserver(proxy, op, context);
            }
            if(observer != null)
            {
                observer.attach();
            }
            return observer;
        }
        return null;
    }

    private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<String, String>();
}
