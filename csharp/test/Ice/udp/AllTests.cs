//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Text;
using Ice.udp.Test;

namespace Ice
{
    namespace udp
    {
        public class AllTests : global::Test.AllTests
        {
            public class PingReplyI : Test.PingReply
            {
                public void reply(Ice.Current current)
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
                        long end = IceInternal.Time.currentMonotonicTimeMillis() + timeout;
                        while (_replies < expectedReplies)
                        {
                            int delay = (int)(end - IceInternal.Time.currentMonotonicTimeMillis());
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

            public static void allTests(global::Test.TestHelper helper)
            {
                Communicator communicator = helper.communicator();
                communicator.SetProperty("ReplyAdapter.Endpoints", "udp");
                ObjectAdapter adapter = communicator.createObjectAdapter("ReplyAdapter");
                PingReplyI replyI = new PingReplyI();
                PingReplyPrx reply = adapter.Add(replyI).Clone(invocationMode: InvocationMode.Datagram);
                adapter.Activate();

                Console.Out.Write("testing udp... ");
                Console.Out.Flush();
                var obj = TestIntfPrx.Parse("test:" + helper.getTestEndpoint(0, "udp"),
                                            communicator).Clone(invocationMode: InvocationMode.Datagram);

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
                    reply = adapter.Add(replyI).Clone(invocationMode: InvocationMode.Datagram);
                }
                test(ret == true);

                if ((communicator.GetPropertyAsInt("Ice.Override.Compress") ?? 0) == 0)
                {
                    //
                    // Only run this test if compression is disabled, the test expect fixed message size
                    // to be sent over the wire.
                    //
                    byte[] seq = null;
                    try
                    {
                        seq = new byte[1024];
                        while (true)
                        {
                            seq = new byte[seq.Length * 2 + 10];
                            replyI.reset();
                            obj.sendByteSeq(seq, reply);
                            replyI.waitReply(1, 10000);
                        }
                    }
                    catch (Ice.DatagramLimitException)
                    {
                        //
                        // The server's Ice.UDP.RcvSize property is set to 16384, which means that DatagramLimitException
                        // will be throw when try to send a packet bigger than that.
                        //
                        test(seq.Length > 16384);
                    }
                    obj.GetConnection().close(Ice.ConnectionClose.GracefullyWithWait);
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
                        test(!b);
                    }
                    catch (Ice.DatagramLimitException)
                    {
                    }
                    catch (Ice.LocalException ex)
                    {
                        Console.Out.WriteLine(ex);
                        test(false);
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
                    if (IceInternal.AssemblyUtil.isWindows || IceInternal.AssemblyUtil.isMacOS)
                    {
                        endpoint.Append(" --interface \"::1\"");
                    }
                }
                else
                {
                    endpoint.Append("udp -h 239.255.1.1");
                    if (IceInternal.AssemblyUtil.isWindows || IceInternal.AssemblyUtil.isMacOS)
                    {
                        endpoint.Append(" --interface 127.0.0.1");
                    }
                }
                endpoint.Append(" -p ");
                endpoint.Append(helper.getTestPort(10));
                var objMcast = TestIntfPrx.Parse($"test -d:{endpoint}", communicator);

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
                    reply = adapter.Add(replyI).Clone(invocationMode: InvocationMode.Datagram);
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
                obj.GetConnection().setAdapter(adapter);
                objMcast.GetConnection().setAdapter(adapter);
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
                    reply = adapter.Add(replyI).Clone(invocationMode: InvocationMode.Datagram);
                }
                test(ret);
                Console.Out.WriteLine("ok");

                //
                // Sending the replies back on the multicast UDP connection doesn't work for most
                // platform(it works for macOS Leopard but not Snow Leopard, doesn't work on SLES,
                // Windows...). For Windows, see UdpTransceiver constructor for the details. So
                // we don't run this test.
                //
                //         Console.Out.Write("testing udp bi-dir connection... ");
                //         nRetry = 5;
                //         while(nRetry-- > 0)
                //         {
                //             replyI.reset();
                //             objMcast.pingBiDir(reply.Identity);
                //             ret = replyI.waitReply(5, 2000);
                //             if(ret)
                //             {
                //                 break; // Success
                //             }
                //             replyI = new PingReplyI();
                //             reply =(PingReplyPrx)PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
                //         }

                //         if(!ret)
                //         {
                //             Console.Out.WriteLine("failed(is a firewall enabled?)");
                //         }
                //         else
                //         {
                //             Console.Out.WriteLine("ok");
                //         }
            }
        }
    }
}
