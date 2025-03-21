// Copyright (c) ZeroC, Inc.

using System.Text;

namespace Ice.udp;

public class AllTests : global::Test.AllTests
{
    public class PingReplyI : Test.PingReplyDisp_
    {
        public override void reply(Ice.Current current)
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
                long end = Ice.Internal.Time.currentMonotonicTimeMillis() + timeout;
                while (_replies < expectedReplies)
                {
                    int delay = (int)(end - Ice.Internal.Time.currentMonotonicTimeMillis());
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

    public static async Task allTests(global::Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        communicator.getProperties().setProperty("ReplyAdapter.Endpoints", "udp");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ReplyAdapter");
        PingReplyI replyI = new PingReplyI();
        Test.PingReplyPrx reply =
          (Test.PingReplyPrx)Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        adapter.activate();

        Console.Out.Write("testing udp... ");
        Console.Out.Flush();
        Ice.ObjectPrx @base = communicator.stringToProxy("test:" + helper.getTestEndpoint(0, "udp")).ice_datagram();
        Test.TestIntfPrx obj = Test.TestIntfPrxHelper.uncheckedCast(@base);

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
            reply = (Test.PingReplyPrx)Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        }
        test(ret == true);

        if (communicator.getProperties().getIcePropertyAsInt("Ice.Override.Compress") == 0)
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
                    seq = new byte[(seq.Length * 2) + 10];
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

            await obj.ice_getConnection().closeAsync();

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
        if (communicator.getProperties().getIceProperty("Ice.IPv6") == "1")
        {
            endpoint.Append("udp -h \"ff15::1:1\"");
            if (Ice.Internal.AssemblyUtil.isWindows || Ice.Internal.AssemblyUtil.isMacOS)
            {
                endpoint.Append(" --interface \"::1\"");
            }
        }
        else
        {
            endpoint.Append("udp -h 239.255.1.1");
            if (Ice.Internal.AssemblyUtil.isWindows || Ice.Internal.AssemblyUtil.isMacOS)
            {
                endpoint.Append(" --interface 127.0.0.1");
            }
        }
        endpoint.Append(" -p ");
        endpoint.Append(helper.getTestPort(10));
        @base = communicator.stringToProxy("test -d:" + endpoint.ToString());
        var objMcast = Test.TestIntfPrxHelper.uncheckedCast(@base);

        nRetry = 5;
        while (nRetry-- > 0)
        {
            replyI.reset();
            try
            {
                objMcast.ping(reply);
            }
            catch (Ice.SocketException)
            {
                if (communicator.getProperties().getIceProperty("Ice.IPv6") == "1")
                {
                    // Multicast IPv6 not supported on the platform. This occurs for example on macOS big_suir
                    Console.Out.Write("(not supported) ");
                    ret = true;
                    break;
                }
                throw;
            }
            ret = replyI.waitReply(5, 5000);
            if (ret)
            {
                break;
            }
            replyI = new PingReplyI();
            reply = (Test.PingReplyPrx)Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        }
        test(ret);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing udp bi-dir connection... ");
        // This feature is only half-implemented. In particular, we maintain a single Connection object on the server
        // side that gets updated each time we receive a new request.
        Console.Out.Flush();
        obj.ice_getConnection().setAdapter(adapter);
        nRetry = 5;
        while (nRetry-- > 0)
        {
            replyI.reset();
            obj.pingBiDir(reply.ice_getIdentity());
            obj.pingBiDir(reply.ice_getIdentity());
            obj.pingBiDir(reply.ice_getIdentity());
            ret = replyI.waitReply(3, 2000);
            if (ret)
            {
                break; // Success
            }
            replyI = new PingReplyI();
            reply = (Test.PingReplyPrx)Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        }
        test(ret);
        Console.Out.WriteLine("ok");
    }
}
