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
        /// transport to send other queued requests. When the sending of the request completes, this method returns
        /// the stream created to send the request. The stream can be used to receive the response or for stream
        /// parameters.</summary>
        /// <param name="frame">The request frame to send.</param>
        /// <param name="bidirectional">Indicates if the request should create a bi-directional stream.</param>
        /// <param name="observer">The invocation observer.</param>
        /// <param name="cancel">The cancellation token to cancel the sending of the request</param>
        /// <returns>The stream to receive the response or stream additional data.</returns>
        ValueTask<Stream> SendRequestAsync(
            OutgoingRequestFrame frame,
            bool bidirectional,
            IInvocationObserver? observer,
            CancellationToken cancel);
    }
}
