// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    public int
    run(String[] args)
    {
        args = communicator().getProperties().parseCommandLineOptions("Discover", args);

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("DiscoverReply");
        DiscoverReplyI replyI = new DiscoverReplyI();
        DiscoverReplyPrx reply = DiscoverReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI));
        adapter.activate();

        DiscoverPrx discover = DiscoverPrxHelper.uncheckedCast(
            communicator().propertyToProxy("Discover.Proxy").ice_datagram());
        discover.lookup(reply);
        Ice.ObjectPrx base = replyI.waitReply(2000);

        if(base == null)
        {
            System.err.println(appName() + ": no replies");
            return 1;
        }
        HelloPrx hello = HelloPrxHelper.checkedCast(base);
        if(hello == null)
        {
            System.err.println(appName() + ": invalid reply");
            return 1;
        }

        hello.sayHello();
        return 0;
    }

    public static void
    main(String[] args)
    {
        Client app = new Client();
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }
}

