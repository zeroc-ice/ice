//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>An invoker is a delegate used to send an outgoing request to an InvocationInterceptor
    /// or to a proxy.</summary>
    /// <param name="target">The proxy for the invocation.</param>
    /// <param name="request">The outgoing request being send.</param>
    /// <returns>The response frame.</returns>
    public delegate ValueTask<IncomingResponseFrame> Invoker(IObjectPrx target, OutgoingRequestFrame request);

    /// <summary>A invocation interceptor can be registered with the Communicator to intercept proxy invocations.
    /// </summary>
    /// <param name="target">The proxy for the invocation.</param>
    /// <param name="request">The outgoing request being send.</param>
    /// <param name="next">The next invoker in the invocation chain, an interceptor implementation should invoke
    /// this delegate to continue with the operation invocation.</param>
    /// <returns>The response frame.</returns>
    public delegate ValueTask<IncomingResponseFrame> InvocationInterceptor(
        IObjectPrx target,
        OutgoingRequestFrame request,
        Invoker next);
}
