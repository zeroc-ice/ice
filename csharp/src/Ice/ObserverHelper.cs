//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;
    using Ice.Instrumentation;

    public sealed class ObserverHelper
    {
        public static IInvocationObserver? get(Ice.Communicator communicator, string op)
        {
            ICommunicatorObserver? obsv = communicator.Observer;
            if (obsv != null)
            {
                IInvocationObserver? observer = obsv.getInvocationObserver(null, op, _emptyContext);
                if (observer != null)
                {
                    observer.attach();
                }
                return observer;
            }
            return null;
        }

        public static IInvocationObserver? get(Ice.IObjectPrx proxy, string op)
        {
            return get(proxy, op, null);
        }

        public static IInvocationObserver? get(Ice.IObjectPrx proxy, string op, Dictionary<string, string>? context)
        {
            ICommunicatorObserver? obsv = proxy.Communicator.Observer;
            if (obsv != null)
            {
                IInvocationObserver observer;
                if (context == null)
                {
                    observer = obsv.getInvocationObserver(proxy, op, _emptyContext);
                }
                else
                {
                    observer = obsv.getInvocationObserver(proxy, op, context);
                }
                if (observer != null)
                {
                    observer.attach();
                }
                return observer;
            }
            return null;
        }

        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
    }
}
