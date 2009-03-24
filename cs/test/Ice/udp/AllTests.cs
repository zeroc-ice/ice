// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
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
                Monitor.Pulse(this);
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
                        Monitor.Wait(this, delay);
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

    public static Test.TestIntfPrx allTests(Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("ReplyAdapter.Endpoints", "udp -p 12030");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ReplyAdapter");
        PingReplyI replyI = new PingReplyI();
        Test.PingReplyPrx reply = 
            (Test.PingReplyPrx)Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI)).ice_datagram();
        adapter.activate();

        Console.Out.Write("testing udp... ");
        Console.Out.Flush();
        Ice.ObjectPrx @base = communicator.stringToProxy("test:udp -p 12010").ice_datagram();
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
                test(seq.Length > 16384);
            }
            
            communicator.getProperties().setProperty("Ice.UDP.SndSize", "64000");
            seq = new byte[50000];
            try
            {
                replyI.reset();
                obj.sendByteSeq(seq, reply);
                test(!replyI.waitReply(1, 500));
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
        String host;
        if(communicator.getProperties().getProperty("Ice.IPv6") == "1")
        {
            host = "\"ff01::1:1\"";
        }
        else
        {
            host = "239.255.1.1";
        }
        @base = communicator.stringToProxy("test:udp -h " + host + " -p 12020").ice_datagram();
        obj = Test.TestIntfPrxHelper.uncheckedCast(@base);

        replyI.reset();
        obj.ping(reply);
        if(!replyI.waitReply(5, 5000))
        {
            Console.Out.WriteLine("failed (is a firewall enabled?)");
            return obj;
        }

        replyI.reset();
        obj.ping(reply);
        if(!replyI.waitReply(5, 5000))
        {
            Console.Out.WriteLine("failed (is a firewall enabled?)");
            return obj;
        }

        Console.Out.WriteLine("ok");

        return obj;
    }
}
