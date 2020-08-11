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
    public interface ITransceiver : IAsyncDisposable
    {
        Socket? Socket { get; }
        void CheckSendSize(int size);
        ValueTask ClosingAsync(Exception exception, CancellationToken cancel);
        ValueTask InitializeAsync(CancellationToken cancel);
        ValueTask<ArraySegment<byte>> ReceiveAsync(CancellationToken cancel);
        ValueTask<int> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel);
        ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel);
        string ToDetailedString();
    }
}
