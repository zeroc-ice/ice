// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
                long end = System.DateTime.Now.Ticks / 1000 + timeout;
                while(_replies < expectedReplies)
                {
                    int delay = (int)(end - System.DateTime.Now.Ticks / 1000);
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

        replyI.reset();
        obj.ping(reply);
        obj.ping(reply);
        obj.ping(reply);
        bool ret = replyI.waitReply(3, 2000);
        test(ret == true);
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
        ret = replyI.waitReply(5, 5000);
        test(ret == true);

        replyI.reset();
        obj.ping(reply);
        ret = replyI.waitReply(5, 5000);
        test(ret == true);
        Console.Out.WriteLine("ok");

        return obj;
    }
}
