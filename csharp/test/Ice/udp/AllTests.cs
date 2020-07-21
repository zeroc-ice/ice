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
            private readonly object _mutex = new object();

            public void Reply(Current current)
            {
                lock (_mutex)
                {
                    ++_replies;
                    System.Threading.Monitor.Pulse(_mutex);
                }
            }

            public void Reset()
            {
                lock (_mutex)
                {
                    _replies = 0;
                }
            }

            public bool WaitReply(int expectedReplies, TimeSpan timeout)
            {
                lock (_mutex)
                {
                    TimeSpan end = Time.Elapsed + timeout;
                    while (_replies < expectedReplies)
                    {
                        TimeSpan delay = end - Time.Elapsed;
                        if (delay > TimeSpan.Zero)
                        {
                            System.Threading.Monitor.Wait(_mutex, delay);
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
                int val = obj.GetValue();
                TestHelper.Assert(false);
            }
            catch (InvalidOperationException)
            {
                // expected
            }

            int nRetry = 5;
            bool ret = false;
            while (nRetry-- > 0)
            {
                replyI.Reset();
                obj.Ping(reply);
                obj.Ping(reply);
                obj.Ping(reply);
                ret = replyI.WaitReply(3, TimeSpan.FromSeconds(2));
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
                        replyI.Reset();
                        obj.SendByteSeq(seq, reply);
                        replyI.WaitReply(1, TimeSpan.FromSeconds(10));
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
                communicator.SetProperty("Ice.UDP.SndSize", "64K");
                seq = new byte[50000];
                try
                {
                    replyI.Reset();
                    obj.SendByteSeq(seq, reply);

                    bool b = replyI.WaitReply(1, TimeSpan.FromMilliseconds(500));
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
                replyI.Reset();
                objMcast.Ping(reply);
                ret = replyI.WaitReply(5, TimeSpan.FromSeconds(5));
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
                replyI.Reset();
                obj.PingBiDir(reply.Identity);
                obj.PingBiDir(reply.Identity);
                obj.PingBiDir(reply.Identity);
                ret = replyI.WaitReply(3, TimeSpan.FromSeconds(2));
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
