// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceMulticastClient")]
[assembly: AssemblyDescription("Ice multicast demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        public override int run(string[] args)
        {
            args = communicator().getProperties().parseCommandLineOptions("Discover", args);

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
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}

