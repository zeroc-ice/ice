// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public class Client : Ice.Application
{
    public override int run(string[] args)
    {
        if(args.Length > 0)
        {
            System.Console.Error.WriteLine(appName() + ": too many arguments");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("DiscoverReply");
        DiscoverReplyI replyI = new DiscoverReplyI();
        DiscoverReplyPrx reply = DiscoverReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI));
        adapter.activate();

        DiscoverPrx discover = DiscoverPrxHelper.uncheckedCast(
            communicator().propertyToProxy("Discover.Proxy").ice_datagram());
        discover.lookup(reply);
        Ice.ObjectPrx obj = replyI.waitReply(2000);


        if(obj == null)
        {
            System.Console.Error.WriteLine(appName() + ": no replies");
            return 1;
        }
        HelloPrx hello = HelloPrxHelper.checkedCast(obj);
        if(hello == null)
        {
            System.Console.Error.WriteLine(appName() + ": invalid reply");
            return 1;
        }

        hello.sayHello();
        return 0;
    }

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}

