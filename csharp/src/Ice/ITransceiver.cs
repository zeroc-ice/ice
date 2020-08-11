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
        Socket? Fd();
        ValueTask InitializeAsync(CancellationToken cancel);
        ValueTask ClosingAsync(Exception exception, CancellationToken cancel);
        ValueTask<int> WriteAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel);
        ValueTask<ArraySegment<byte>> ReadAsync(CancellationToken cancel);
        ValueTask<int> ReadAsync(ArraySegment<byte> buffer, CancellationToken cancel);
        string ToDetailedString();
        void CheckSendSize(int size);
    }
}
