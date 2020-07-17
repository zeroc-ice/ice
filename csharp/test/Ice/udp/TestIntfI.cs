//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.UDP
{
    public sealed class TestIntf : ITestIntf
    {
        public int GetValue(Current current)
        {
            TestHelper.Assert(false); // a two-way operation cannot be reached through UDP
            return 42;
        }

        public void Ping(IPingReplyPrx? reply, Current current)
        {
            try
            {
                reply!.Reply();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }
        }

        public void SendByteSeq(byte[] seq, IPingReplyPrx? reply, Current current)
        {
            try
            {
                reply!.Reply();
            }
            catch
            {
                TestHelper.Assert(false);
            }
        }

        public void PingBiDir(Identity id, Current current)
        {
            try
            {
                //
                // Ensure sending too much data doesn't cause the UDP connection
                // to be closed.
                //
                TestHelper.Assert(current.Connection != null);
                try
                {
                    byte[] seq = new byte[32 * 1024];
                    current.Connection.CreateProxy(id, ITestIntfPrx.Factory).SendByteSeq(seq, null);
                }
                catch (DatagramLimitException)
                {
                    // Expected.
                }

                current.Connection.CreateProxy(id, IPingReplyPrx.Factory).Reply();
            }
            catch
            {
                TestHelper.Assert(false);
            }
        }

        public void Shutdown(Current current) => _ = current.Adapter.Communicator.ShutdownAsync();
    }
}
