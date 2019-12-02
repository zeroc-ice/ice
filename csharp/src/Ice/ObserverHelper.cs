//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;
    using Ice.Instrumentation;

    public sealed class ObserverHelper
    {
        public static InvocationObserver get(Ice.Communicator communicator, string op)
        {
            CommunicatorObserver obsv = communicator.initializationData().observer;
            if (obsv != null)
            {
                InvocationObserver observer = obsv.getInvocationObserver(null, op, _emptyContext);
                if (observer != null)
                {
                    observer.attach();
                }
                return observer;
            }
            return null;
        }

        public static InvocationObserver get(Ice.IObjectPrx proxy, string op)
        {
            return get(proxy, op, null);
        }

        public static InvocationObserver get(Ice.IObjectPrx proxy, string op, Dictionary<string, string> context)
        {
            CommunicatorObserver obsv = proxy.Communicator.initializationData().observer;
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
