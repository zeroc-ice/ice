//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Diagnostics;
using System.Collections.Generic;

namespace Ice.perf
{
    public sealed class PerformanceI : Test.IPerformance
    {
        private static byte[] _bytes = new byte[Test.Constants.ByteSeqSize];

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
