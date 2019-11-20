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
            public void ping(Test.PingReplyPrx reply, Ice.Current current)
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

            public void sendByteSeq(byte[] seq, Test.PingReplyPrx reply, Ice.Current current)
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
                        Test.TestIntfPrxHelper.uncheckedCast(current.con.createProxy(id)).sendByteSeq(seq, null);
                    }
                    catch (Ice.DatagramLimitException)
                    {
                        // Expected.
                    }

                    Test.PingReplyPrxHelper.uncheckedCast(current.con.createProxy(id)).reply();
                }
                catch (Ice.LocalException)
                {
                    Debug.Assert(false);
                }
            }

            public void shutdown(Ice.Current current)
            {
                current.adapter.GetCommunicator().shutdown();
            }
        }
    }
}
