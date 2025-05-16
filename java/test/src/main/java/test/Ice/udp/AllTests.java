// Copyright (c) ZeroC, Inc.

package test.Ice.udp;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.DatagramLimitException;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.SocketException;

import test.Ice.udp.Test.PingReply;
import test.Ice.udp.Test.PingReplyPrx;
import test.Ice.udp.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static class PingReplyI implements PingReply {
        @Override
        public synchronized void reply(Current current) {
            ++_replies;
            notify();
        }

        public synchronized void reset() {
            _replies = 0;
        }

        public synchronized boolean waitReply(int expectedReplies, long timeout, PrintWriter out) {
            long end = System.currentTimeMillis() + timeout;
            while (_replies < expectedReplies) {
                long delay = end - System.currentTimeMillis();
                if (delay > 0) {
                    try {
                        wait(delay);
                    } catch (InterruptedException ex) {}
                } else {
                    break;
                }
            }
            if (_replies != expectedReplies) {
                out.println("Expected " + expectedReplies + " replies, got " + _replies);
            }
            return _replies == expectedReplies;
        }

        private int _replies;
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        communicator.getProperties().setProperty("ReplyAdapter.Endpoints", "udp");
        ObjectAdapter adapter = communicator.createObjectAdapter("ReplyAdapter");
        PingReplyI replyI = new PingReplyI();

        PingReplyPrx reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        adapter.activate();

        out.print("testing udp... ");
        out.flush();
        ObjectPrx base =
            communicator.stringToProxy("test -d:" + helper.getTestEndpoint(0, "udp"));
        TestIntfPrx obj = TestIntfPrx.uncheckedCast(base);

        int nRetry = 5;
        boolean ret = false;
        while (nRetry-- > 0) {
            replyI.reset();
            obj.ping(reply);
            obj.ping(reply);
            obj.ping(reply);
            ret = replyI.waitReply(3, 2000, out);
            if (ret) {
                break; // Success
            }

            // If the 3 datagrams were not received within the 2 seconds, we try again to receive 3
            // new datagrams using a new object. We give up after 5 retries.
            replyI = new PingReplyI();
            reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        }
        test(ret == true);

        if (communicator.getProperties().getIcePropertyAsInt("Ice.Override.Compress") == 0) {
            //
            // Only run this test if compression is disabled, the test expects fixed message size to
            // be sent over the wire.
            //
            byte[] seq = null;
            try {
                seq = new byte[1024];
                while (true) {
                    seq = new byte[seq.length * 2 + 10];
                    replyI.reset();
                    obj.sendByteSeq(seq, reply);
                    replyI.waitReply(1, 10000, out);
                }
            } catch (DatagramLimitException ex) {
                test(seq.length > 16384);
            }

            communicator.getProperties().setProperty("Ice.UDP.SndSize", "64000");
            obj.ice_getConnection().close();
            seq = new byte[50000];
            try {
                replyI.reset();
                obj.sendByteSeq(seq, reply);
                //
                // We don't expect a reply because the server's value for Ice.UDP.RcvSize is too
                // small.
                //
                test(!replyI.waitReply(1, 500, out));
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
        }

        out.println("ok");

        out.print("testing udp multicast... ");
        out.flush();
        {
            StringBuilder endpoint = new StringBuilder();
            if ("1".equals(communicator.getProperties().getIceProperty("Ice.IPv6"))) {
                endpoint.append("udp -h \"ff15::1:1\" -p ");
                endpoint.append(helper.getTestPort(communicator.getProperties(), 10));
                if (System.getProperty("os.name").contains("OS X")
                    || System.getProperty("os.name").startsWith("Windows")) {
                    endpoint.append(
                        " --interface \"::1\""); // Use loopback to prevent other machines to
                    // answer.
                }
            } else {
                endpoint.append("udp -h 239.255.1.1 -p ");
                endpoint.append(helper.getTestPort(communicator.getProperties(), 10));
                if (System.getProperty("os.name").contains("OS X")
                    || System.getProperty("os.name").startsWith("Windows")) {
                    endpoint.append(
                        " --interface 127.0.0.1"); // Use loopback to prevent other machines to
                    // answer.
                }
            }
            base = communicator.stringToProxy("test -d:" + endpoint.toString());
            TestIntfPrx objMcast = TestIntfPrx.uncheckedCast(base);

            //
            // On Android, the test suite driver only starts one server instance. Otherwise, we
            // expect there to be five servers and we expect a response from all of them.
            //
            final int numServers = helper.isAndroid() ? 1 : 5;

            nRetry = 5;
            while (nRetry-- > 0) {
                replyI.reset();
                try {
                    objMcast.ping(reply);
                } catch (SocketException ex) {
                    if ("1".equals(communicator.getProperties().getIceProperty("Ice.IPv6"))) {
                        // Multicast IPv6 not supported on the platform. This occurs for example on
                        // macOS Big Sur
                        out.print("(not supported) ");
                        ret = true;
                        break;
                    }
                    throw ex;
                }
                ret = replyI.waitReply(numServers, 2000, out);
                if (ret) {
                    break; // Success
                }
                replyI = new PingReplyI();
                reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
            }

            test(ret);
            out.println("ok");

            out.print("testing udp bi-dir connection... ");
            // This feature is only half-implemented. In particular, we maintain a single
            // Connection object on the server side that gets updated each time we receive
            // a new request.
            out.flush();
            obj.ice_getConnection().setAdapter(adapter);
            nRetry = 5;
            while (nRetry-- > 0) {
                replyI.reset();
                obj.pingBiDir(reply.ice_getIdentity());
                obj.pingBiDir(reply.ice_getIdentity());
                obj.pingBiDir(reply.ice_getIdentity());
                ret = replyI.waitReply(3, 2000, out);
                if (ret) {
                    break; // Success
                }
                replyI = new PingReplyI();
                reply = PingReplyPrx.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
            }
            test(ret);
        }
        out.println("ok");
    }

    private AllTests() {}
}
