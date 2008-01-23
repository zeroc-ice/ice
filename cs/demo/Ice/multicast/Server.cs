// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        if(args.Length > 0)
        {
            System.Console.Error.WriteLine(appName() + ": too many arguments");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");
        Ice.ObjectAdapter discoverAdapter = communicator().createObjectAdapter("Discover");

        Ice.ObjectPrx hello = adapter.addWithUUID(new HelloI());
        discoverAdapter.add(new DiscoverI(hello), communicator().stringToIdentity("discover"));

        discoverAdapter.activate();        
        adapter.activate();
        
        communicator().waitForShutdown();
        return 0;
    }

    public static void Main(string[] args)
    {
        Server app = new Server();
        int status = app.main(args, "config.server");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
