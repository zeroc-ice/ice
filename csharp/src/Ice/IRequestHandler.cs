//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    /// <summary>The request handler interface is used by the proxy code to send requests and get a response task in
    /// return to wait for the reception of the response for twoway requests. This interface has two concrete
    /// implementations: the connection request handler which hands over the request to an Ice network connection and
    /// the collocated request handler which instead directly invokes the request on the collocated servant.</summary>
    internal interface IRequestHandler
    {
        /// <summary>Sends a request. The sending is asynchronous because it might block waiting for the underlying
        /// transport to send other queued requests. When the sending of the request completes, this method eventually
        /// returns another Task for waiting the response. If the request is a oneway request, the returned task is
        /// null. If it's a twoway request, a non-null task is returned and the caller should use this task to wait for
        /// the receipt of the response).</summary>
        /// <param name="frame">The request frame to send.</param>
        /// <param name="oneway">Indicates if the request is a oneway request or not.</param>
        /// <param name="synchronous">Indicates whether or not the request is synchronous or asynchronous. This is
        /// useful for the collocated request handler implementation to figure out whether or not the request can be
        /// invoked on the user thread or not.</param>
        /// <param name="observer">The invocation observer.</param>
        /// <returns>A task if the request is a twoway request or null if it's a oneway request. The returned task can
        /// be used to wait for the receipt of the response.</returns>
        ValueTask<Task<IncomingResponseFrame>?> SendRequestAsync(OutgoingRequestFrame frame, bool oneway,
            bool synchronous, IInvocationObserver? observer);

        /// <summary>Returns the Ice network connection associated with the request handler if any.</summary>
        /// <returns>The connection or null if the request handler doesn't use a connection.</returns>
        Connection? GetConnection();
    }
}
