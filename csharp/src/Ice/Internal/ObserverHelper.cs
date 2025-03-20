// Copyright (c) ZeroC, Inc.

using Ice.Instrumentation;

namespace Ice.Internal;

public sealed class ObserverHelper
{
    public static InvocationObserver get(Instance instance, string op)
    {
        CommunicatorObserver obsv = instance.initializationData().observer;
        if (obsv != null)
        {
            InvocationObserver observer = obsv.getInvocationObserver(null, op, _emptyContext);
            observer?.attach();
            return observer;
        }
        return null;
    }

    public static InvocationObserver get(Ice.ObjectPrx proxy, string op) => get(proxy, op, null);

    public static InvocationObserver get(Ice.ObjectPrx proxy, string op, Dictionary<string, string> context)
    {
        CommunicatorObserver obsv =
            ((Ice.ObjectPrxHelperBase)proxy).iceReference().getInstance().initializationData().observer;
        if (obsv != null)
        {
            InvocationObserver observer;
            if (context == null)
            {
                observer = obsv.getInvocationObserver(proxy, op, _emptyContext);
            }
            else
            {
                observer = obsv.getInvocationObserver(proxy, op, context);
            }
            observer?.attach();
            return observer;
        }
        return null;
    }

    private static readonly Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
}
