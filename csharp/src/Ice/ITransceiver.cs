//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A transceiver represents a connection between too peer.</summary>
    public interface ITransceiver : IAsyncDisposable
    {
        /// <summary>Gets the optional socket associated with this transceiver.</summary>
        Socket? Socket { get; }

        /// <summary>Checks if the transceiver can send messages of the given size. Throw if the message is too large.
        /// </summary>
        /// <param name="size">The size of the message to check.</param>
        // TODO: Remove this? this is only used for datagrams.
        void CheckSendSize(int size);

        /// <summary>Closes the transceiver. The transceiver might use this method to send a notification to the peer
        /// of the connection closure.</summary>
        /// <param name="exception">The reason of the connection closure.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        ValueTask ClosingAsync(Exception exception, CancellationToken cancel);

        /// <summary>Initializes the transceiver. The transceiver might use this method to establish or accept the
        /// connection.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        ValueTask InitializeAsync(CancellationToken cancel);

        /// <summary>Receives data from the connection. This is used for datagram connections only. The
        /// transceiver is responsible for allocating the memory for the returned data.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The received data.</return>
        ValueTask<ArraySegment<byte>> ReceiveAsync(CancellationToken cancel);

        /// <summary>Receives data from the connection. This is used for stream based connections only. The data
        /// is received and copied in the given buffer.</summary>
        /// <param name="buffer">The buffer to use to copy the received data.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The number of bytes received.</return>
        ValueTask<int> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel);

        /// <summary>Receive data from the connection.</summary>
        /// <param name="buffer">The buffer containing the data to send.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The number of bytes sent.</return>
        ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel);

        /// <summary>Gets a detailed description of the connection.</summary>
        /// <return>The detailed description.</return>
        string ToDetailedString();
    }
}
