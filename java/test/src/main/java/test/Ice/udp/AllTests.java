// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.udp;

import test.Ice.udp.Test.*;

import java.io.PrintWriter;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static class PingReplyI implements PingReply
    {
        @Override
        public synchronized void reply(com.zeroc.Ice.Current current)
        {
            ++_replies;
            notify();
        }

        public synchronized void reset()
        {
             _replies = 0;
        }

        public synchronized boolean waitReply(int expectedReplies, long timeout)
        {
            long end = System.currentTimeMillis() + timeout;
            while(_replies < expectedReplies)
            {
                long delay = end - System.currentTimeMillis();
                if(delay > 0)
                {
                    try
                    {
                        wait(delay);
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                else
                {
                    break;
                }
            }
            return _replies == expectedReplies;
        }

        private int _replies;
    }

    public static void allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        communicator.getProperties().setProperty("ReplyAdapter.Endpoints", "udp");
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ReplyAdapter");
        PingReplyI replyI = new PingReplyI();

        PingReplyPrx reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        adapter.activate();

        out.print("testing udp... ");
        out.flush();
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy("test -d:" + app.getTestEndpoint(0, "udp"));
        TestIntfPrx obj = TestIntfPrx.uncheckedCast(base);

        int nRetry = 5;
        boolean ret = false;
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
            reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        }
        test(ret == true);

        if(communicator.getProperties().getPropertyAsInt("Ice.Override.Compress") == 0)
        {
            //
            // Only run this test if compression is disabled, the test expects fixed message size
            // to be sent over the wire.
            //
            byte[] seq = null;
            try
            {
                seq = new byte[1024];
                while(true)
                {
                    seq = new byte[seq.length * 2 + 10];
                    replyI.reset();
                    obj.sendByteSeq(seq, reply);
                    replyI.waitReply(1, 10000);
                }
            }
            catch(com.zeroc.Ice.DatagramLimitException ex)
            {
                test(seq.length > 16384);
            }

            communicator.getProperties().setProperty("Ice.UDP.SndSize", "25000");
            obj.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
            seq = new byte[24000];
            try
            {
                replyI.reset();
                obj.sendByteSeq(seq, reply);
                //
                // We don't expect a reply because the server's value for Ice.UDP.RcvSize is too small.
                //
                test(!replyI.waitReply(1, 500));
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false);
            }
        }

        out.println("ok");

        out.print("testing udp multicast... ");
        out.flush();
        {
            StringBuilder endpoint = new StringBuilder();
            if(communicator.getProperties().getProperty("Ice.IPv6").equals("1"))
            {
                endpoint.append("udp -h \"ff15::1:1\" -p ");
                endpoint.append(app.getTestPort(communicator.getProperties(), 10));
                if(System.getProperty("os.name").contains("OS X") ||
                   System.getProperty("os.name").startsWith("Windows"))
                {
                    endpoint.append(" --interface \"::1\""); // Use loopback to prevent other machines to answer.
                }
            }
            else
            {
                endpoint.append("udp -h 239.255.1.1 -p ");
                endpoint.append(app.getTestPort(communicator.getProperties(), 10));
                if(System.getProperty("os.name").contains("OS X") ||
                   System.getProperty("os.name").startsWith("Windows"))
                {
                    endpoint.append(" --interface 127.0.0.1"); // Use loopback to prevent other machines to answer.
                }
            }
            base = communicator.stringToProxy("test -d:" + endpoint.toString());
            TestIntfPrx objMcast = TestIntfPrx.uncheckedCast(base);

            //
            // On Android, the test suite driver only starts one server instance. Otherwise, we expect
            // there to be five servers and we expect a response from all of them.
            //
            final int numServers = app.isAndroid() ? 1 : 5;

            nRetry = 5;
            while(nRetry-- > 0)
            {
                replyI.reset();
                objMcast.ping(reply);
                ret = replyI.waitReply(numServers, 2000);
                if(ret)
                {
                    break; // Success
                }
                replyI = new PingReplyI();
                reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
            }
            if(!ret)
            {
                out.println("failed (is a firewall enabled?)");
            }
            else
            {
                out.println("ok");
            }

            out.print("testing udp bi-dir connection... ");
            out.flush();
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
                reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
            }
            test(ret);
        }
        out.println("ok");

        //
        // Sending the replies back on the multicast UDP connection doesn't work for most
        // platform (it works for macOS Leopard but not Snow Leopard, doesn't work on SLES,
        // Windows...). For Windows, see UdpTransceiver constructor for the details. So
        // we don't run this test.
        //
//         out.print("testing udp bi-dir connection... ");
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
//             reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
//         }

//         if(!ret)
//         {
//             out.println("failed (is a firewall enabled?)");
//         }
//         else
//         {
//             out.println("ok");
//         }
    }
}
