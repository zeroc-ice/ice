//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

namespace ZeroC.Ice.Test.Perf
{
    public sealed class PerformanceI : IPerformance
    {
        private static byte[] _bytes = new byte[Constants.ByteSeqSize];

        public void sendBytes(byte[] seq, Current current)
        {
        }

        public System.ReadOnlyMemory<byte> receiveBytes(int size, Current current)
        {
            Debug.Assert(size <= _bytes.Length);
            return new System.ReadOnlyMemory<byte>(_bytes, 0, size);
        }

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
