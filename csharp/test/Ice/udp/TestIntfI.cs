//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

namespace Ice
{
    namespace udp
    {
        public sealed class TestIntfI : Test.TestIntf
        {
            public void ping(Test.IPingReplyPrx reply, Ice.Current current)
            {
                try
                {
                    reply.reply();
                }
                catch (Ice.LocalException)
                {
                    Debug.Assert(false);
                }
            }

            public void sendByteSeq(byte[] seq, Test.IPingReplyPrx reply, Ice.Current current)
            {
                try
                {
                    reply.reply();
                }
                catch (Ice.LocalException)
                {
                    Debug.Assert(false);
                }
            }

            public void pingBiDir(Ice.Identity id, Ice.Current current)
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
                catch (Ice.LocalException)
                {
                    Debug.Assert(false);
                }
            }

            public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
        }
    }
}
