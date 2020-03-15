//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.Instrumentation;

using ReadOnlyContext = System.Collections.Generic.IReadOnlyDictionary<string, string>;

namespace IceInternal
{
    internal static class ObserverHelper
    {
        internal static IInvocationObserver? GetInvocationObserver(Ice.IObjectPrx proxy, string op,
                                                                   ReadOnlyContext context)
        {
            if (proxy.Communicator.Observer is ICommunicatorObserver communicatorObserver)
            {
                IInvocationObserver? observer = communicatorObserver.GetInvocationObserver(proxy, op, context);
                observer?.Attach();
                return observer;
            }
            return null;
        }
    }
}
