// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Net.Security;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A single-stream socket represents the local end of a network connection and enables transmitting
    /// raw binary data over a transport such as TCP, UDP or WebSocket.</summary>
    public abstract class SingleStreamSocket : IDisposable
    {
        /// <summary>Gets the optional .NET socket associated with this single-stream socket.</summary>
        public abstract Socket? Socket { get; }

        /// <summary>Gets the optional SslStream associated with this socket.</summary>
        public abstract SslStream? SslStream { get; }

        /// <summary>Checks if the socket can send messages of the given size. Throw if the message is too large.
        /// </summary>
        /// <param name="size">The size of the message to check.</param>
        // TODO: Remove this? This is used to ensure the user doesn't try to send a message which is larger
        // than the datagram size... UDP users would be better of not sending large datagrams instead of us
        // trying to softly error if the user sends a large datagram.
        public virtual void CheckSendSize(int size)
        {
        }

        /// <summary>Closes the socket. The socket might use this method to send a notification to the peer
        /// of the connection closure.</summary>
        /// <param name="exception">The reason of the connection closure.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public abstract ValueTask CloseAsync(Exception exception, CancellationToken cancel);

        /// <summary>Releases the resources used by the socket.</summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>Initializes the socket. This is called to initialize the socket during connection establishment
        /// or when a new connection is accepted.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public abstract ValueTask InitializeAsync(CancellationToken cancel);

        /// <summary>Receives a new datagram from the connection, only supported for datagram connections.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The received data.</return>
        public abstract ValueTask<ArraySegment<byte>> ReceiveDatagramAsync(CancellationToken cancel);

        /// <summary>Receives data from the connection. This is used for stream based connections only.</summary>
        /// <param name="buffer">The buffer that holds the received data.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The number of bytes received.</return>
        public abstract ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel);

        /// <summary>Send data over the connection.</summary>
        /// <param name="buffer">The buffer containing the data to send.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The number of bytes sent.</return>
        public abstract ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel);

        /// <summary>Releases the resources used by the socket.</summary>
        /// <param name="disposing">True to release both managed and unmanaged resources; false to release only
        /// unmanaged resources.</param>
        protected abstract void Dispose(bool disposing);
    }
}
