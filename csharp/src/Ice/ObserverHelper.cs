//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Ice.Instrumentation;

namespace IceInternal
{
    public sealed class ObserverHelper
    {
        public static IInvocationObserver? get(Ice.Communicator communicator, string op)
        {
            ICommunicatorObserver? obsv = communicator.Observer;
            if (obsv != null)
            {
                IInvocationObserver? observer = obsv.GetInvocationObserver(null, op, _emptyContext);
                if (observer != null)
                {
                    observer.Attach();
                }
                return observer;
            }
            return null;
        }

        public static IInvocationObserver? get(Ice.IObjectPrx proxy, string op) => get(proxy, op, null);

        public static IInvocationObserver? get(Ice.IObjectPrx proxy, string op, Dictionary<string, string>? context)
        {
            ICommunicatorObserver? obsv = proxy.Communicator.Observer;
            if (obsv != null)
            {
                IInvocationObserver? observer;
                if (context == null)
                {
                    observer = obsv.GetInvocationObserver(proxy, op, _emptyContext);
                }
                else
                {
                    observer = obsv.GetInvocationObserver(proxy, op, context);
                }
                if (observer != null)
                {
                    observer.Attach();
                }
                return observer;
            }
            return null;
        }

        private static readonly Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
    }
}
