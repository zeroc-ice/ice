// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Net.Security;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A transceiver enables transmitting and receiving raw binary data over a transport such as TCP,
    /// UDP, TLS or WebSocket. More transports can be supported by implementing this interface.</summary>
    public interface ITransceiver : IDisposable
    {
        /// <summary>Gets the optional socket associated with this transceiver.</summary>
        Socket? Socket { get; }

        /// <summary>Gets the optional SslStream associated with this transceiver.</summary>
        SslStream? SslStream { get; }

        /// <summary>Checks if the transceiver can send messages of the given size. Throw if the message is too large.
        /// </summary>
        /// <param name="size">The size of the message to check.</param>
        // TODO: Remove this? This is used to ensure the user doesn't try to send a message which is larger
        // than the datagram size... UDP users would be better of not sending large datagrams instead of us
        // trying to softly error if the user sends a large datagram.
        void CheckSendSize(int size);

        /// <summary>Closes the transceiver. The transceiver might use this method to send a notification to the peer
        /// of the connection closure.</summary>
        /// <param name="exception">The reason of the connection closure.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        ValueTask CloseAsync(Exception exception, CancellationToken cancel);

        /// <summary>Initializes the transceiver. This is called by the connection during initialization to initialize
        /// the transceiver.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        ValueTask InitializeAsync(CancellationToken cancel);

        /// <summary>Receives a new datagram from the connection, only supported for datagram connections.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The received data.</return>
        ValueTask<ArraySegment<byte>> ReceiveDatagramAsync(CancellationToken cancel);

        /// <summary>Receives data from the connection. This is used for stream based connections only.</summary>
        /// <param name="buffer">The buffer that holds the received data.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The number of bytes received.</return>
        ValueTask<int> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel);

        /// <summary>Receive data from the connection.</summary>
        /// <param name="buffer">The buffer containing the data to send.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The number of bytes sent.</return>
        ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel);
    }
}
