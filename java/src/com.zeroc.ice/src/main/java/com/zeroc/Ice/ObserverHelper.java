// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.Instrumentation.InvocationObserver;

final class ObserverHelper {
    public static InvocationObserver get(Instance instance, String op) {
        CommunicatorObserver obsv = instance.initializationData().observer;
        if (obsv != null) {
            InvocationObserver observer = obsv.getInvocationObserver(null, op, _emptyContext);
            if (observer != null) {
                observer.attach();
            }
            return observer;
        }
        return null;
    }

    public static InvocationObserver get(ObjectPrx proxy, String op) {
        return get(proxy, op, null);
    }

    public static InvocationObserver get(
            ObjectPrx proxy, String op, java.util.Map<String, String> context) {
        CommunicatorObserver obsv =
                ((_ObjectPrxI) proxy)._getReference().getInstance().initializationData().observer;
        if (obsv != null) {
            InvocationObserver observer;
            if (context == null) {
                observer = obsv.getInvocationObserver(proxy, op, _emptyContext);
            } else {
                observer = obsv.getInvocationObserver(proxy, op, context);
            }
            if (observer != null) {
                observer.attach();
            }
            return observer;
        }
        return null;
    }

    private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<>();
}
