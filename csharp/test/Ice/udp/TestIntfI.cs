// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using ZeroC.Test;

namespace ZeroC.Ice.Test.UDP
{
    public sealed class TestIntf : ITestIntf
    {
        public int GetValue(Current current, CancellationToken cancel)
        {
            TestHelper.Assert(false); // a two-way operation cannot be reached through UDP
            return 42;
        }

        public void Ping(IPingReplyPrx reply, Current current, CancellationToken cancel)
        {
            try
            {
                reply.Clone(preferNonSecure: NonSecure.Always).Reply(cancel: cancel);
            }
            catch
            {
                TestHelper.Assert(false);
            }
        }

        public void SendByteSeq(byte[] seq, IPingReplyPrx? reply, Current current, CancellationToken cancel)
        {
            try
            {
                reply?.Clone(preferNonSecure: NonSecure.Always).Reply(cancel: cancel);
            }
            catch
            {
                TestHelper.Assert(false);
            }
        }

        public void PingBiDir(Identity id, Current current, CancellationToken cancel)
        {
            try
            {
                // Ensure sending too much data doesn't cause the UDP connection to be closed.
                TestHelper.Assert(current.Connection != null);
                try
                {
                    byte[] seq = new byte[64 * 1024];
                    current.Connection.CreateProxy(id, "", ITestIntfPrx.Factory).SendByteSeq(seq, null, cancel: cancel);
                }
                catch (TransportException)
                {
                    // Expected.
                }

                current.Connection.CreateProxy(id, "", IPingReplyPrx.Factory).Reply(cancel: cancel);
            }
            catch (System.Exception ex)
            {
                TestHelper.Assert(false, ex.ToString());
            }
        }

        public void Shutdown(Current current, CancellationToken cancel) =>
            _ = current.Communicator.ShutdownAsync();
    }
}
