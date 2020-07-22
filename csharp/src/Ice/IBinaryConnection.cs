//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    public interface IBinaryConnection : IDisposable, IAsyncDisposable
    {
        Endpoint Endpoint { get; }
        ITransceiver Transceiver { get; }

        /// <summary>Gracefully closes the transport.</summary>
        ValueTask CloseAsync(Exception exception, CancellationToken cancel);

        void IDisposable.Dispose()
        {
            GC.SuppressFinalize(this);
            DisposeAsync().AsTask().Wait();
        }

        /// <summary>Sends a heartbeat.</summary>
        ValueTask HeartbeatAsync(CancellationToken cancel);

        /// <summary>Initializes the transport.</summary>
        ValueTask InitializeAsync(
            Action heartbeatCallback,
            Action<int> sentCallback,
            Action<int> receivedCallback,
            CancellationToken cancel);

        /// <summary>Receives a new frame.</summary>
        ValueTask<(int StreamId, object? Frame, bool Fin)> ReceiveAsync(CancellationToken cancel);

        /// <summary>Creates a new stream.</summary>
        int NewStream(bool bidirectional);

        /// <summary>Resets the given stream.</summary>
        ValueTask ResetAsync(int streamId);

        /// <summary>Sends the given frame on an existing stream.</summary>
        ValueTask SendAsync(int streamId, object frame, bool fin, CancellationToken cancel);
    }
}
