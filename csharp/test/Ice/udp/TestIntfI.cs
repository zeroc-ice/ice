//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

namespace Ice.udp
{
    public sealed class TestIntf : Test.ITestIntf
    {
        public void ping(Test.IPingReplyPrx reply, Current current)
        {
            try
            {
                reply.reply();
            }
            catch (LocalException)
            {
                Debug.Assert(false);
            }
        }

        public void sendByteSeq(byte[] seq, Test.IPingReplyPrx reply, Current current)
        {
            try
            {
                reply.reply();
            }
            catch (LocalException)
            {
                Debug.Assert(false);
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
                try
                {
                    byte[] seq = new byte[32 * 1024];
                    Test.ITestIntfPrx.UncheckedCast(current.Connection.createProxy(id)).sendByteSeq(seq, null);
                }
                catch (Ice.DatagramLimitException)
                {
                    // Expected.
                }

                Test.IPingReplyPrx.UncheckedCast(current.Connection.createProxy(id)).reply();
            }
            catch (LocalException)
            {
                Debug.Assert(false);
            }
        }

        public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
    }
}
