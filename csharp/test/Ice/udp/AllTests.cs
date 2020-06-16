//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Text;
using Test;

namespace ZeroC.Ice.Test.UDP
{
    public class AllTests
    {
        public class PingReplyI : IPingReply
        {
            public void reply(Current current)
            {
                lock (this)
                {
                    ++_replies;
                    System.Threading.Monitor.Pulse(this);
                }
            }

            public void reset()
            {
                lock (this)
                {
                    _replies = 0;
                }
            }

            public bool waitReply(int expectedReplies, long timeout)
            {
                lock (this)
                {
                    long end = Time.CurrentMonotonicTimeMillis() + timeout;
                    while (_replies < expectedReplies)
                    {
                        int delay = (int)(end - Time.CurrentMonotonicTimeMillis());
                        if (delay > 0)
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

        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            communicator.SetProperty("ReplyAdapter.Endpoints", "udp");
            ObjectAdapter adapter = communicator.CreateObjectAdapter("ReplyAdapter");
            PingReplyI replyI = new PingReplyI();
            IPingReplyPrx reply = adapter.AddWithUUID(replyI, IPingReplyPrx.Factory)
                .Clone(invocationMode: InvocationMode.Datagram);
            adapter.Activate();

            Console.Out.Write("testing udp... ");
            Console.Out.Flush();
            var obj = ITestIntfPrx.Parse("test:" + helper.GetTestEndpoint(0, "udp"),
                                        communicator).Clone(invocationMode: InvocationMode.Datagram);

            try
            {
                int val = obj.getValue();
                TestHelper.Assert(false);
            }
            catch (System.InvalidOperationException)
            {
                // expected
            }

            int nRetry = 5;
            bool ret = false;
            while (nRetry-- > 0)
            {
                replyI.reset();
                obj.ping(reply);
                obj.ping(reply);
                obj.ping(reply);
                ret = replyI.waitReply(3, 2000);
                if (ret)
                {
                    break; // Success
                }

                // If the 3 datagrams were not received within the 2 seconds, we try again to
                // receive 3 new datagrams using a new object. We give up after 5 retries.
                replyI = new PingReplyI();
                reply = adapter.AddWithUUID(
                    replyI, IPingReplyPrx.Factory).Clone(invocationMode: InvocationMode.Datagram);
            }
            TestHelper.Assert(ret == true);

            if (!(communicator.GetPropertyAsBool("Ice.Override.Compress") ?? false))
            {
                //
                // Only run this test if compression is disabled, the test expect fixed message size
                // to be sent over the wire.
                //
                byte[] seq = new byte[1024];
                try
                {
                    while (true)
                    {
                        seq = new byte[seq.Length * 2 + 10];
                        replyI.reset();
                        obj.sendByteSeq(seq, reply);
                        replyI.waitReply(1, 10000);
                    }
                }
                catch (DatagramLimitException)
                {
                    //
                    // The server's Ice.UDP.RcvSize property is set to 16384, which means that DatagramLimitException
                    // will be throw when try to send a packet bigger than that.
                    //
                    TestHelper.Assert(seq.Length > 16384);
                }
                obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                communicator.SetProperty("Ice.UDP.SndSize", "64000");
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
                    TestHelper.Assert(!b);
                }
                catch (DatagramLimitException)
                {
                }
                catch (Exception ex)
                {
                    Console.Out.WriteLine(ex);
                    TestHelper.Assert(false);
                }
            }

            Console.Out.WriteLine("ok");

            Console.Out.Write("testing udp multicast... ");
            Console.Out.Flush();
            StringBuilder endpoint = new StringBuilder();
            //
            // Use loopback to prevent other machines to answer.
            //
            if (communicator.GetProperty("Ice.IPv6") == "1")
            {
                endpoint.Append("udp -h \"ff15::1:1\"");
                if (AssemblyUtil.IsWindows || AssemblyUtil.IsMacOS)
                {
                    endpoint.Append(" --interface \"::1\"");
                }
            }
            else
            {
                endpoint.Append("udp -h 239.255.1.1");
                if (AssemblyUtil.IsWindows || AssemblyUtil.IsMacOS)
                {
                    endpoint.Append(" --interface 127.0.0.1");
                }
            }
            endpoint.Append(" -p ");
            endpoint.Append(helper.GetTestPort(10));
            var objMcast = ITestIntfPrx.Parse($"test -d:{endpoint}", communicator);

            nRetry = 5;
            while (nRetry-- > 0)
            {
                replyI.reset();
                objMcast.ping(reply);
                ret = replyI.waitReply(5, 5000);
                if (ret)
                {
                    break;
                }
                replyI = new PingReplyI();
                reply = adapter.AddWithUUID(
                    replyI, IPingReplyPrx.Factory).Clone(invocationMode: InvocationMode.Datagram);
            }
            if (!ret)
            {
                Console.Out.WriteLine("failed(is a firewall enabled?)");
            }
            else
            {
                Console.Out.WriteLine("ok");
            }

            Console.Out.Write("testing udp bi-dir connection... ");
            Console.Out.Flush();
            obj.GetConnection()!.Adapter = adapter;
            objMcast.GetConnection()!.Adapter = adapter;
            nRetry = 5;
            while (nRetry-- > 0)
            {
                replyI.reset();
                obj.pingBiDir(reply.Identity);
                obj.pingBiDir(reply.Identity);
                obj.pingBiDir(reply.Identity);
                ret = replyI.waitReply(3, 2000);
                if (ret)
                {
                    break; // Success
                }
                replyI = new PingReplyI();
                reply = adapter.AddWithUUID(
                    replyI, IPingReplyPrx.Factory).Clone(invocationMode: InvocationMode.Datagram);
            }
            TestHelper.Assert(ret);
            Console.Out.WriteLine("ok");
        }
    }
}
