//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.udp
{
    public sealed class TestIntf : Test.ITestIntf
    {
        public int getValue(Current current)
        {
            TestHelper.Assert(false); // a two-way operation cannot be reached through UDP
            return 42;
        }

        public void ping(Test.IPingReplyPrx? reply, Current current)
        {
            try
            {
                reply!.reply();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }
        }

        public void sendByteSeq(byte[] seq, Test.IPingReplyPrx? reply, Current current)
        {
            try
            {
                reply!.reply();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }
        }

        public void pingBiDir(Identity id, Current current)
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
                    current.Connection.CreateProxy(id, Test.ITestIntfPrx.Factory).sendByteSeq(seq, null);
                }
                catch (DatagramLimitException)
                {
                    // Expected.
                }

                current.Connection.CreateProxy(id, Test.IPingReplyPrx.Factory).reply();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }
        }

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
