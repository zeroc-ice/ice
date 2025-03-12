// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.udp
{
    public sealed class TestIntfI : Test.TestIntfDisp_
    {
        public override void ping(Test.PingReplyPrx reply, Ice.Current current)
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

        public override void sendByteSeq(byte[] seq, Test.PingReplyPrx reply, Ice.Current current)
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

        public override void pingBiDir(Ice.Identity id, Ice.Current current)
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

        public override void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }
}
