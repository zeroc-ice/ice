// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Diagnostics;
using System.Threading;

namespace ZeroC.Ice.Test.Perf
{
    public sealed class PerformanceI : IPerformance
    {
        private static readonly byte[] _bytes = new byte[Constants.ByteSeqSize];

        public void SendBytes(byte[] seq, Current current, CancellationToken cancel)
        {
        }

        public System.ReadOnlyMemory<byte> ReceiveBytes(int size, Current current, CancellationToken cancel)
        {
            Debug.Assert(size <= _bytes.Length);
            return new(_bytes, 0, size);
        }

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();
    }
}
