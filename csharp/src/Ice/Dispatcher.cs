// Copyright (c) ZeroC, Inc. All rights reserved.
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A dispatcher is a delegate that dispatches an incoming request to a dispatch interceptor or to a
    /// servant.</summary>
    /// <param name="request">The incoming request being dispatched.</param>
    /// <param name="current">The current object for the dispatch.</param>
    /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is cancelled.
    /// </param>
    /// <returns>The outgoing response frame.</returns>
    public delegate ValueTask<OutgoingResponseFrame> Dispatcher(
        IncomingRequestFrame request,
        Current current,
        CancellationToken cancel);

    /// <summary>A dispatch interceptor can be registered with the Communicator or with an ObjectAdapter to intercept
    /// operation dispatches.</summary>
    /// <param name="request">The incoming request being dispatched.</param>
    /// <param name="current">The current object for the dispatch.</param>
    /// <param name="next">The next dispatcher in the dispatch chain.</param>
    /// <param name="cancel">A cancellation token that is notified of cancellation when the dispatch is cancelled.
    /// </param>
    /// <returns>The outgoing response frame.</returns>
    public delegate ValueTask<OutgoingResponseFrame> DispatchInterceptor(
        IncomingRequestFrame request,
        Current current,
        Dispatcher next,
        CancellationToken cancel);
}
