// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Text;
using System.Threading;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public class PingReplyI : Test.PingReplyDisp_
    {
        public override void reply(Ice.Current current)
        {
            lock(this)
            {
                ++_replies;
                System.Threading.Monitor.Pulse(this);
            }
        }

        public void reset()
        {
            lock(this)
            {
                 _replies = 0;
            }
        }

        public bool waitReply(int expectedReplies, long timeout)
        {
            lock(this)
            {
                long end = IceInternal.Time.currentMonotonicTimeMillis() + timeout;
                while(_replies < expectedReplies)
                {
                    int delay = (int)(end - IceInternal.Time.currentMonotonicTimeMillis());
                    if(delay > 0)
                    {
                        System.Threading.Monitor.Wait(this, delay);
                    }
                    else
                    {
                        break;
                    }
                }
                return _replies == expectedReplies;
            }
        }

        private int _replies = 0;
    }

    public static void allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        communicator.getProperties().setProperty("ReplyAdapter.Endpoints", "udp");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ReplyAdapter");
        PingReplyI replyI = new PingReplyI();
        Test.PingReplyPrx reply =
            (Test.PingReplyPrx)Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        adapter.activate();

        Console.Out.Write("testing udp... ");
        Console.Out.Flush();
        Ice.ObjectPrx @base = communicator.stringToProxy("test:" + app.getTestEndpoint(0, "udp")).ice_datagram();
        Test.TestIntfPrx obj = Test.TestIntfPrxHelper.uncheckedCast(@base);

        int nRetry = 5;
        bool ret = false;
        while(nRetry-- > 0)
        {
            replyI.reset();
            obj.ping(reply);
            obj.ping(reply);
            obj.ping(reply);
            ret = replyI.waitReply(3, 2000);
            if(ret)
            {
                break; // Success
            }

            // If the 3 datagrams were not received within the 2 seconds, we try again to
            // receive 3 new datagrams using a new object. We give up after 5 retries.
            replyI = new PingReplyI();
            reply =(Test.PingReplyPrx)Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        }
        test(ret == true);

        if(communicator.getProperties().getPropertyAsInt("Ice.Override.Compress") == 0)
        {
            //
            // Only run this test if compression is disabled, the test expect fixed message size
            // to be sent over the wire.
            //
            byte[] seq = null;
            try
            {
                seq = new byte[1024];
                while(true)
                {
                    seq = new byte[seq.Length * 2 + 10];
                    replyI.reset();
                    obj.sendByteSeq(seq, reply);
                    replyI.waitReply(1, 10000);
                }
            }
            catch(Ice.DatagramLimitException)
            {
                //
                // The server's Ice.UDP.RcvSize property is set to 16384, which means that DatagramLimitException
                // will be throw when try to send a packet bigger than that.
                //
                test(seq.Length > 16384);
            }
            obj.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            communicator.getProperties().setProperty("Ice.UDP.SndSize", "64000");
            seq = new byte[50000];
            try
            {
                replyI.reset();
                obj.sendByteSeq(seq, reply);

                bool b = replyI.waitReply(1, 500);
                //
                // The server's Ice.UDP.RcvSize property is set to 16384, which means this packet
                // should not be delivered.
                //
                test(!b);
            }
            catch(Ice.DatagramLimitException)
            {
            }
            catch(Ice.LocalException ex)
            {
                Console.Out.WriteLine(ex);
                test(false);
            }
        }

        Console.Out.WriteLine("ok");

        Console.Out.Write("testing udp multicast... ");
        Console.Out.Flush();
        StringBuilder endpoint = new StringBuilder();
        if(communicator.getProperties().getProperty("Ice.IPv6").Equals("1"))
        {
            endpoint.Append("udp -h \"ff15::1:1\" -p ");
        }
        else
        {
            endpoint.Append("udp -h 239.255.1.1 -p ");
        }
        endpoint.Append(app.getTestPort(10));
        @base = communicator.stringToProxy("test -d:" + endpoint.ToString());
        TestIntfPrx objMcast = Test.TestIntfPrxHelper.uncheckedCast(@base);

        nRetry = 5;
        while(nRetry-- > 0)
        {
            replyI.reset();
            objMcast.ping(reply);
            ret = replyI.waitReply(5, 5000);
            if(ret)
            {
                break;
            }
            replyI = new PingReplyI();
            reply =(Test.PingReplyPrx)Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        }
        if(!ret)
        {
            Console.Out.WriteLine("failed (is a firewall enabled?)");
        }
        else
        {
            Console.Out.WriteLine("ok");
        }

        Console.Out.Write("testing udp bi-dir connection... ");
        Console.Out.Flush();
        obj.ice_getConnection().setAdapter(adapter);
        objMcast.ice_getConnection().setAdapter(adapter);
        nRetry = 5;
        while(nRetry-- > 0)
        {
            replyI.reset();
            obj.pingBiDir(reply.ice_getIdentity());
            obj.pingBiDir(reply.ice_getIdentity());
            obj.pingBiDir(reply.ice_getIdentity());
            ret = replyI.waitReply(3, 2000);
            if(ret)
            {
                break; // Success
            }
            replyI = new PingReplyI();
            reply = (PingReplyPrx)PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        }
        test(ret);
        Console.Out.WriteLine("ok");

        //
        // Sending the replies back on the multicast UDP connection doesn't work for most
        // platform (it works for macOS Leopard but not Snow Leopard, doesn't work on SLES,
        // Windows...). For Windows, see UdpTransceiver constructor for the details. So
        // we don't run this test.
        //
//         Console.Out.Write("testing udp bi-dir connection... ");
//         nRetry = 5;
//         while(nRetry-- > 0)
//         {
//             replyI.reset();
//             objMcast.pingBiDir(reply.ice_getIdentity());
//             ret = replyI.waitReply(5, 2000);
//             if(ret)
//             {
//                 break; // Success
//             }
//             replyI = new PingReplyI();
//             reply = (PingReplyPrx)PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
//         }

//         if(!ret)
//         {
//             Console.Out.WriteLine("failed (is a firewall enabled?)");
//         }
//         else
//         {
//             Console.Out.WriteLine("ok");
//         }
    }
}
